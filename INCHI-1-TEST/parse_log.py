import json
import sys
from pathlib import Path
from difflib import SequenceMatcher
from collections import defaultdict
from typing import Callable, Final, TextIO
from sdf_pipeline.utils import select_records_from_gzipped_sdf
from .config import get_args, DATASETS


def _get_html_diff(current: str, reference: str) -> str:
    html_diff: str = ""
    seq_matcher = SequenceMatcher(None, current, reference)

    for opcode, cur_beg, cur_end, ref_beg, ref_end in seq_matcher.get_opcodes():
        # returns a sequence of instructions for how to convert current into reference
        match opcode:
            case "equal":
                html_diff += current[cur_beg:cur_end]
            case "insert":
                html_diff += f"<ins style='background-color: #d4fcbc;'>{reference[ref_beg:ref_end]}</ins>"
            case "delete":
                html_diff += f"<del style='background-color: #fbb;'>{current[cur_beg:cur_end]}</del>"
            case "replace":
                html_diff += f"<del style='background-color: #fbb;'>{current[cur_beg:cur_end]}</del>"
                html_diff += f"<ins style='background-color: #d4fcbc;'>{reference[ref_beg:ref_end]}</ins>"
            case _:
                raise RuntimeError("Unknown opcode.")

    return html_diff


def _build_molfile_log_invariance(log_entry: dict) -> dict:
    molfile_log: dict = {}

    molfile_log["time"] = log_entry["time"]
    molfile_log["parameters"] = log_entry["info"]["parameters"]
    molfile_log["variants"] = log_entry["variants"]

    return molfile_log


def _build_molfile_log_regression(log_entry: dict) -> dict:
    molfile_log: dict = {}

    molfile_log["time"] = log_entry["time"]
    molfile_log["parameters"] = log_entry["info"]["parameters"]
    molfile_log["current"] = {}
    molfile_log["reference"] = {}

    diff = log_entry["diff"]
    current = json.loads(diff["current"])
    reference = json.loads(diff["reference"])

    for key, current_value in current.items():
        reference_value = reference[key]
        if current_value != reference_value:
            molfile_log["current"][key] = current_value
            molfile_log["reference"][key] = reference_value

    return molfile_log


_build_molfile_log: Final[dict[str, Callable]] = {
    "invariance": _build_molfile_log_invariance,
    "regression": _build_molfile_log_regression,
}


def _write_html_entry_invariance(molfile_log: dict, html_file: TextIO) -> None:
    variants = molfile_log["variants"]

    for attribute in variants[0]:
        html_file.write(f"<h3>{attribute}</h3>\n")
        html_file.write("<table>\n")

        for i, value in enumerate([variant[attribute] for variant in variants]):
            html_file.write("<tr>\n")
            if attribute == "molfile":
                html_file.write("<details>\n")
                html_file.write(f"<summary>{i + 1}.</summary>\n")
                html_file.write(
                    f"<pre style='display: inline-block; border: 1px solid black; border-radius: 10px; padding: 10px'>{value}</pre>\n"
                )
                html_file.write("</details>\n")
            else:
                html_file.write(f"<td>{i + 1}.: {value}</td>\n")
            html_file.write("</tr>\n")
        html_file.write("<table>\n")

    return None


def _write_html_entry_regression(molfile_log: dict, html_file: TextIO) -> None:
    current: dict = molfile_log["current"]
    reference: dict = molfile_log["reference"]

    for key, current_value in current.items():
        reference_value = reference[key]
        html_file.write(f"<h3>{key}</h3>\n")
        html_file.write("<table>\n")
        html_file.write("<tr>\n")
        html_file.write("<td>current:</td>\n")
        html_file.write(f"<td>{current_value}</td>\n")
        html_file.write("</tr>\n")
        html_file.write("<td>reference:</td>\n")
        html_file.write(f"<td>{reference_value}</td>\n")
        html_file.write("<tr>\n")
        html_file.write("<td>difference:</td>\n")
        html_file.write(
            f"<td>{_get_html_diff(str(current_value), str(reference_value))}<td>\n"
        )
        html_file.write("</tr>\n")
        html_file.write("<table>\n")

    return None


_write_html_entry: Final[dict[str, Callable]] = {
    "invariance": _write_html_entry_invariance,
    "regression": _write_html_entry_regression,
}


def _write_html_log(
    sdf_log: dict,
    summary_path: Path,
    sdf_path: Path,
    get_molfile_id: Callable,
    test: str,
) -> None:
    with summary_path.open("w") as html_file:
        html_file.write("<!DOCTYPE html>\n")
        html_file.write("<html>\n")
        html_file.write("<head>\n")
        html_file.write(
            f"<title>InChI {test.upper()} Test Failures {sdf_path.name}</title>\n"
        )
        html_file.write("</head>\n")
        html_file.write("<body>\n")

        molfile_ids = set(sdf_log.keys())
        for molfile_id, molfile in select_records_from_gzipped_sdf(
            sdf_path, molfile_ids, get_molfile_id
        ):
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

            _write_html_entry[test](molfile_log, html_file)

        html_file.write("</body>\n")
        html_file.write("</html>\n")

    return None


if __name__ == "__main__":
    test, dataset = get_args()

    log_paths = sorted(DATASETS[dataset]["log_path"].glob(f"*_{test}_{dataset}.log"))
    if not log_paths:
        print(f"No logs for {test} in {DATASETS[dataset]['log_path']}.")
        sys.exit()

    # Process most recent log file
    # TODO: parametrize with time?
    log_path: Path = log_paths[-1]
    sdf_logs: dict[str, dict] = defaultdict(dict)

    with log_path.open("r") as log_file:
        for line in log_file:
            if not line.startswith(f"INFO:sdf_pipeline:{test} test failed"):
                continue
            log_entry_start = line.index("{")
            log_entry = json.loads(line[log_entry_start:])
            sdf_logs[log_entry["sdf"]][log_entry["molfile_id"]] = _build_molfile_log[
                test
            ](log_entry)

    for sdf, sdf_log in sdf_logs.items():
        sdf_path = DATASETS[dataset]["log_path"].joinpath(sdf)
        _write_html_log(
            sdf_log,
            DATASETS[dataset]["log_path"].joinpath(
                f"{log_path.stem}_{sdf_path.stem}.html"
            ),
            sdf_path,
            DATASETS[dataset]["molfile_id"],
            test,
        )
