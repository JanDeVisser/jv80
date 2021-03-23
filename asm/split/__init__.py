def _split(line: str):

    # Clean the input string by:
    # - Stripping leading and trailing whitespace
    # - Collapsing sequences of spaces and/or tabs into a single space.
    def clean(s):
        def _merge_spaces(s):
            merged = s.replace("  ", " ")
            return _merge_spaces(merged) if merged != s else s

        return _merge_spaces(s.strip().replace("\t", " "))

    # "Clean" the line. clean() will merge consecutive spaces/tabs into a single space.
    c = clean(line)

    # If the line is empty after cleaning, return the empty list. Otherwise split on spaces.
    return c.split() if c != "" else []


def split(line):
    states = []
    ix = 0
    start_quote = 0
    while ix < len(line):
        ch = line[ix]
        if not states:
            if ch in '\'"`':
                start_quote = ix
                states.insert(0, ch)
            elif ch == '\\':
                ix += 1
            elif ch == ';':
                return split(line[:ix])
        elif states[0] == '\\':
            repl = ch
            if line[ix] == 'n':
                repl = '\n'
            elif line[ix] == 't':
                repl = '\t'
            elif line[ix] == 'b':
                repl = '\b'
            line = line[:ix] + repl + line[ix+1:]
            states = states[1:]
        elif states[0] in '"\'':
            if ch == states[0]:
                ret = split(line[:start_quote])
                ret.append(line[start_quote:ix+1])
                ret.extend(split(line[ix+1:]))
                return ret
            elif ch == '\\':
                line = line[:ix] + line[ix+1:]
                ix -= 1
                states.insert(0, '\\')
        ix += 1
    if states and states[0] in '"\'':
        raise ValueError(f"Mismatched quote {states[0]} in {line}")
    return _split(line)


if __name__ == "__main__":
    def test_split(s, expected):
        err: ValueError = None
        parts = []
        try:
            parts = split(s)
        except ValueError as ve:
            err = ve
        if isinstance(expected, list):
            if err is not None:
                print(f"split('{s}'): expected {len(expected)} parts, got ValueError {err}")
                return
            if len(parts) != len(expected):
                print(f"split('{s}'): expected {len(expected)} parts, found {len(parts)}")
                return
            for ix in range(len(parts)):
                if parts[ix] != expected[ix]:
                    print(f"split('{s}'): expected part {ix} to be {expected[ix]}, found {parts[ix]}")
                    return
        elif isinstance(expected, ValueError) and err is None:
            print(f"split('{s}'): expected ValueError, got {len(parts)} parts")
            return
        print(f"split('{s}') SUCCESS")

    test_split("db '\\n'", [ "db", "'\n'" ])
    test_split("db '\\d'", [ "db", "'d'" ])
    test_split("db '\\\\'", [ "db", "'\\'" ])
    test_split("db 'jan", ValueError())
    test_split("db '\\'", ValueError())
