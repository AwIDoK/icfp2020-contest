# 18
def s(x):
    def s1(y):
        def s2(z):
            return x(z)(y(z))
        return s2
    return s1


# 19
def c(x):
    def c1(y):
        def c2(z):
            return x(z)(y)
        return c2

    return c1


# 20
def b(x):
    def b1(y):
        def b2(z):
            return x(y(z))
        return b2

    return b1


#21
def t(x):
    def t1(y):
        return x

    return t1


#22
def f(x):
    def f1(y):
        return y
    return f1


#24
def i(x):
    return x


#25
def cons(x):
    def cons1(y):
        return (x, y)
    return cons1


#26
def car(x):
    return x[0]


#27
def cdr(x):
    return x[1]


#28
nil = ()


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
