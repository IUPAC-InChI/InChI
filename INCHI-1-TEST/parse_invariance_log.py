import json
import sys
from pathlib import Path
from .config import get_dataset_arg, select_molfiles_from_sdf, DATASETS


def write_html_log(sdf_log: dict, summary_path: Path, sdf_path: Path) -> None:
    with summary_path.open("w") as html_file:
        html_file.write("<!DOCTYPE html>\n")
        html_file.write("<html>\n")
        html_file.write("<head>\n")
        html_file.write(
            f"<title>InChI Invariance Test Failures {sdf_path.name}</title>\n"
        )
        html_file.write("</head>\n")
        html_file.write("<body>\n")

        molfile_ids = set(sdf_log.keys())
        for molfile_id, molfile in select_molfiles_from_sdf(sdf_path, molfile_ids):
            molfile_log: dict = sdf_log[molfile_id]
            html_file.write(f"<h1>{molfile_id}</h1>\n")

            html_file.write("<details>\n")
            html_file.write("<summary>metadata</summary>\n")
            html_file.write(f"<p>time: {molfile_log['time']}\n")
            html_file.write(
                f"<p>arguments to MakeINCHIFromMolfileText: {molfile_log['parameters']}\n"
            )
            html_file.write("</details>\n")

            html_file.write("<details>\n")
            html_file.write("<summary>SDF</summary>\n")
            html_file.write(
                f"<pre style='display: inline-block; border: 1px solid black; border-radius: 10px; padding: 10px'>{molfile}</pre>\n"
            )
            html_file.write("</details>\n")

            variants = molfile_log["variants"]
            for attribute in variants[0]:
                html_file.write(f"<h3>{attribute}</h3>\n")
                html_file.write("<table>\n")
                for i, value in enumerate([variant[attribute] for variant in variants]):
                    html_file.write("<tr>\n")
                    html_file.write(f"<td>{i + 1}.: {value}</td>\n")
                    html_file.write("</tr>\n")
                html_file.write("<table>\n")

        html_file.write("</body>\n")
        html_file.write("</html>\n")

    return None


if __name__ == "__main__":
    dataset = get_dataset_arg()

    log_paths = sorted(
        DATASETS[dataset]["log_path"].glob(f"*_invariance_{dataset}.log")
    )
    if not log_paths:
        print(f"There aren't any logs in {DATASETS[dataset]['log_path']}")
        sys.exit()

    log_path: Path = log_paths[-1]  # process most recent log file; TODO: parametrize?
    previous_sdf: str = ""
    sdf_log: dict = {}

    with log_path.open("r") as log_file:
        for line in log_file:
            if not line.startswith("INFO:sdf_pipeline:invariance test failed:"):
                continue
            log_entry = json.loads(
                line.lstrip("INFO:sdf_pipeline:invariance test failed:")
            )

            if previous_sdf:
                if previous_sdf != log_entry["sdf"]:
                    sdf_path = DATASETS[dataset]["log_path"].joinpath(previous_sdf)
                    write_html_log(
                        sdf_log,
                        DATASETS[dataset]["log_path"].joinpath(
                            f"{log_path.stem}_{sdf_path.stem}.html"
                        ),
                        sdf_path,
                    )
                    sdf_log = {}
            previous_sdf = log_entry["sdf"]

            molfile_log: dict = {}
            molfile_log["time"] = log_entry["time"]
            molfile_log["parameters"] = log_entry["info"]["parameters"]
            molfile_log["variants"] = log_entry["variants"]

            sdf_log[log_entry["molfile_id"]] = molfile_log

        if previous_sdf:
            # take care of last or only SDF
            sdf_path = DATASETS[dataset]["log_path"].joinpath(previous_sdf)
            write_html_log(
                sdf_log,
                DATASETS[dataset]["log_path"].joinpath(
                    f"{log_path.stem}_{sdf_path.stem}.html"
                ),
                sdf_path,
            )
