import sys

sys.setrecursionlimit(100000)

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


def neg(x):
    return -x


def isnil(x):
    return t if x == () else f


def eq(a, b):
    return t if a == b else f


def mul(a, b):
    return a * b


def add(a, b):
    return a + b


def lt(a, b):
    return t if a < b else f


def div(a, b):
    #TODO
    return int(float(a)/b)


def evaluate(term, function_dict):
    if isinstance(term, Ap):
        return term.evaluate(Ap)
    if isinstance(term, str):
        return function_dict[term]
    return term


class Ap:
    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def evaluate(self, function_dict):
        lhs = self.lhs
        rhs = self.rhs
        if isinstance(lhs, str):
            lhs = evaluate(function_dict[lhs])
        if isinstance(rhs, str):
            rhs = evaluate(function_dict[rhs])
        return lhs(rhs)


def is_int(string):
    if string[0] == "-":
        return string[1:].isdigit()
    return string.isdigit()


def parse(tokens):
    tok = tokens[0]
    rem = tokens[1:]
    if tok == "ap":
        lhs, cont1 = parse(rem)
        rhs, cont2 = parse(cont1)
        return Ap(lhs, rhs), cont2
    elif is_int(tok):
        return int(tok), rem
    elif tok[0] == ":":
        return tok, rem
    elif tok == "nil":
        return (), rem
    else:
        if tok in globals():
            return globals()[tok], rem
        print("FAIL", tok)
        exit()
        return tok, rem


def main():
    function_dict = {}
    for line in sys.stdin:
        if line[-1] == "\n":
            line = line[:-1]
        split = line.split("=")
        name = split[0].strip()
        parsed = parse(split[1].strip().split(" "))
        assert(parsed[1] == [])
        function_dict[name] = parsed[0]

    print(evaluate(function_dict["galaxy"], function_dict))


if __name__ == "__main__":
    main()
    
