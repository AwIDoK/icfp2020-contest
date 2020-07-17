def cons(x):
    def f(y):
        return (x, y)
    return f


def parse(tokens):
    tok = tokens[0]
    rem = tokens[1:]
    if tok == "ap":
        lhs, cont1 = parse(rem)
        rhs, cont2 = parse(cont1)
        return lhs(rhs), cont2
    elif tok == "cons":
        return cons, rem
    else:
        return tok, rem


if __name__ == "__main__":
    print(parse(input().split()))
