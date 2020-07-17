def decode(encoded: str) -> (str, str):
    prefix = encoded[0:2]
    remainder = encoded[2:]
    if prefix == "11":
        # list
        a, cont = decode(remainder)
        b, cont2 = decode(cont)
        return "(" + a + "," + b + ")", cont2
    elif prefix == "01":
        # positive
        pos = remainder.find("0")
        if pos == 0:
            return "0", remainder[1:]
        else:
            number = remainder[pos + 1: pos + 1 + 4 * pos]
            return str(int(number, 2)), remainder[pos + 1 + 4 * pos:]
    elif prefix == "10":
        # negative
        pos = remainder.find("0")
        if pos == 0:
            return "0", remainder[1:]
        else:
            number = remainder[pos + 1: pos + 1 + 4 * pos]
            return str(-int(number, 2)), remainder[pos + 1 + 4 * pos:]
    elif prefix == "00":
        # empty list
        return "()", remainder


if __name__ == "__main__":
    print(decode(input()))
