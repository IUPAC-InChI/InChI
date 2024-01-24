"""`INCHI-1-SRC/INCHI_API/demos/python_sample` stripped to the essentials."""
import ctypes


class InChIOutput(ctypes.Structure):
    # zero-terminated C-strings allocated by GetINCHI()
    # to deallocate all of them call FreeINCHI() (see below)
    _fields_ = [
        ("szInChI", ctypes.POINTER(ctypes.c_char)),
        ("szAuxInfo", ctypes.POINTER(ctypes.c_char)),
        ("szMessage", ctypes.POINTER(ctypes.c_char)),
        ("szLog", ctypes.POINTER(ctypes.c_char)),
    ]


def make_inchi_from_molfile_text(
    inchi_lib: ctypes.CDLL, molfile: str, inchi_options: str = ""
) -> tuple[int, str, str, str, str]:
    c_molfile = ctypes.c_char_p(molfile.encode("utf-8"))
    c_inchi_options = ctypes.c_char_p(inchi_options.encode("utf-8"))

    szInChI = ctypes.create_string_buffer(1)
    szAuxInfo = ctypes.create_string_buffer(1)
    szMessage = ctypes.create_string_buffer(1)
    szLog = ctypes.create_string_buffer(1)
    c_inchi_output = InChIOutput(szInChI, szAuxInfo, szMessage, szLog)

    exit_code = inchi_lib.MakeINCHIFromMolfileText(
        c_molfile, c_inchi_options, ctypes.byref(c_inchi_output)
    )

    inchi = (
        ctypes.cast(c_inchi_output.szInChI, ctypes.c_char_p)
        .value.decode("utf-8")
        .strip()
    )
    log = (
        ctypes.cast(c_inchi_output.szLog, ctypes.c_char_p).value.decode("utf-8").strip()
    )
    message = (
        ctypes.cast(c_inchi_output.szMessage, ctypes.c_char_p)
        .value.decode("utf-8")
        .strip()
    )
    aux = (
        ctypes.cast(c_inchi_output.szAuxInfo, ctypes.c_char_p)
        .value.decode("utf-8")
        .strip()
    )

    # Deallocate memory reallocated by inchi dll.
    # Note the trick with None (Python's NULL) assignment to pointers
    # which have not been allocated (respective strings are zero-length)
    # within C library libinchi.
    # We passed Python buffer_strings casted to char *, not real pointers
    # so they could not be initialized with NULL (as they would be in C).
    # Therefore, if say szLog or szMessage were actually not used in libinchi.dll
    # and were not allocated, they should be messaged to become NULL at the end,
    # in order to avoid unnecessary/illegal freeing upon call in FreeINCHI().
    if not inchi:
        c_inchi_output.szInChI = None
    if not log:
        c_inchi_output.szLog = None
    if not message:
        c_inchi_output.szMessage = None
    if not aux:
        c_inchi_output.szAuxInfo = None

    inchi_lib.FreeINCHI(ctypes.byref(c_inchi_output))

    return exit_code, inchi, log, message, aux


def get_inchi_key_from_inchi(inchi_lib: ctypes.CDLL, inchi: str) -> tuple[int, str]:
    c_inchi = ctypes.c_char_p(inchi.encode("utf-8"))

    szKey = ctypes.create_string_buffer(256)
    szXtra1 = ctypes.create_string_buffer(256)
    szXtra2 = ctypes.create_string_buffer(256)

    exit_code: int = inchi_lib.GetINCHIKeyFromINCHI(
        c_inchi, 0, 0, szKey, szXtra1, szXtra2
    )

    key: str = ""
    if exit_code in [0, 11]:
        # 11: reversal of InChI failed, InChIKey is computed with warning issued
        key = ctypes.cast(szKey, ctypes.c_char_p).value.decode("utf-8").strip()

    return exit_code, key
