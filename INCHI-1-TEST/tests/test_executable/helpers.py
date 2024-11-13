import re


def parse_inchi_from_executable_output(output: str) -> str:
    """The RegEx does not validate the InChI.
    It's meant to be lenient enough to recognize (almost) any InChI in the output.
    See https://chemistry.stackexchange.com/questions/82144/what-is-the-correct-regular-expression-for-inchi.
    """
    inchi_regex = re.compile(r"InChI=1S?/\S+(\s|$)")
    inchi_match = inchi_regex.search(output)
    if inchi_match is None:
        print("Executable output does not contain an InChI.")
        return ""

    return inchi_match.group()


def parse_aux_info_from_executable_output(output: str) -> str:
    aux_info_regex = re.compile(r"AuxInfo=\S+(\s|$)")
    aux_info_match = aux_info_regex.search(output)
    if aux_info_match is None:
        print("Executable output does not contain an AuxInfo.")
        return ""

    return aux_info_match.group()
