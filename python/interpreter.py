import os
import requests
import sys
from PIL import Image

sys.setrecursionlimit(100000)

API_KEY = os.environ['API_KEY']
SEND_URL = "https://icfpc2020-api.testkontur.ru/aliens/send"

def extract(x):
    if isinstance(x, tuple):
        return extract(x[0](())) # should work without extract
    return x


# 18
def s(x_thunk):
    def s1(y_thunk):
        def s2(z_thunk):
            x = extract(x_thunk)
            def ap(x):
                y = extract(y_thunk)
                return y(z_thunk)
            ap_thunk = (ap,)
            return x(z_thunk)(ap_thunk)
        return s2
    return s1


# 19
def c(x_thunk):
    def c1(y_thunk):
        def c2(z_thunk):
            x = extract(x_thunk)
            return x(z_thunk)(y_thunk)
        return c2

    return c1


# 20
def b(x_thunk):
    def b1(y_thunk):
        def b2(z_thunk):
            x = extract(x_thunk)
            def ap(x):
                y = extract(y_thunk)
                return y(z_thunk)
            ap_thunk = (ap,)
            return x(ap_thunk)
        return b2

    return b1


#21
def t(x_thunk):
    def t1(y_thunk):
        return x_thunk

    return t1


#22
def f(x_thunk):
    def f1(y_thunk):
        return y_thunk
    return f1


#24
def i(x_thunk):
    return x_thunk


#25
def cons(x_thunk):
    def cons1(y_thunk):
        def cons2(foo_thunk):
            return extract(foo_thunk)(x_thunk)(y_thunk)
        return cons2
    return cons1


#26
def car(x_thunk):
    return extract(x_thunk)(t)


#27
def cdr(x_thunk):
    return extract(x_thunk)(f)


#28
def nil(x_thunk):
    return t


def neg(x_thunk):
    return -extract(x_thunk)


def isnil(x_thunk):
    def inside(a):
        def inside1(b):
            return f
        return inside1
    return extract(x_thunk)(inside)


def eq(a_thunk):
    def eq1(b_thunk):
        assert(isinstance(extract(a_thunk), int) and isinstance(extract(b_thunk), int))
        return t if extract(a_thunk) == extract(b_thunk) else f

    return eq1


def mul(a_thunk):
    def mul1(b_thunk):
        return extract(a_thunk) * extract(b_thunk)

    return mul1


def add(a_thunk):
    def add1(b_thunk):
        return extract(a_thunk) + extract(b_thunk)
    return add1


def lt(a_thunk):
    def lt1(b_thunk):
        return t if extract(a_thunk) < extract(b_thunk) else f
    return lt1


def div(a_thunk):
    def div1(b_thunk):
        #TODO
        return int(float(extract(a_thunk)) / extract(b_thunk))
    return div1


def send(data):
    params = {
        "apiKey": API_KEY
    }
    response = requests.post(SEND_URL, params=params, data=data)
    assert(response.status_code == 200)
    return response.content.decode("utf-8")


def f38(protocol_thunk, triple_thunk):
    flag = car(triple_thunk)
    newState = car(cdr(triple_thunk))
    data = cdr(cdr(cdr(triple_thunk)))

    if flag == 0:
        return newState, data
    else:
        return interact(protocol_thunk)(newState)(send(data))


def interact(protocol_thunk):
    def interact1(state_thunk):
        def interact2(data_thunk):
            protocol = extract(protocol_thunk)
            return f38(protocol_thunk, protocol(state_thunk)(data_thunk))
        return interact2

    return interact1


def evaluate(term_thunk, function_thunk_dict):
    term = extract(term_thunk)
    if isinstance(term, str):
        return evaluate(function_thunk_dict[term], function_thunk_dict)
    if isinstance(term, Ap):
        return term.evaluate(function_thunk_dict)
    return term


def draw(points):
    max_cell = 15
    cell_size = 60
    side = max_cell * cell_size
    img = Image.new('RGB', (side, side))
    pixels = img.load()
    while isnil(points)(False)(True):
        cur_point = car(points)
        points = cdr(points)
        x, y = car(cur_point), cdr(cur_point)
        assert (x >= 0 and y >= 0 and x < max_cell and y < max_cell)
        for xi in range(x * cell_size, x * cell_size + cell_size):
            for yi in range(y * cell_size, y * cell_size + cell_size):
                pixels[xi, yi] = (255, 255, 255)
    img.show()


def multipledraw(images):
    while isnil(images)(False)(True):
        cur_image = car(images)
        images = cdr(images)
        draw(cur_image)


class Ap:
    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def evaluate(self, function_dict):
        lhs = evaluate(self.lhs, function_dict)
        rhs_thunk = self.rhs
        return lhs(lambda x: evaluate(rhs_thunk, function_dict))


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
        return (lambda x: Ap(lhs, rhs),), cont2
    elif is_int(tok):
        return (lambda x: int(tok),), rem
    elif tok[0] == ":" or tok == 'galaxy':
        return (lambda x: tok,), rem
    else:
        if tok in globals():
            return (lambda x: globals()[tok],), rem
        print("FAIL", tok)
        exit()
        return (lambda x: tok,), rem


def main():
    file = open('galaxy.txt', 'r')
    function_thunk_dict = {}
    for line in file:
        if line[-1] == "\n":
            line = line[:-1]
        split = line.split("=")
        name = split[0].strip()
        parsed = parse(split[1].strip().split(" "))
        assert(parsed[1] == [])
        function_thunk_dict[name] = parsed[0]
    print(evaluate(function_thunk_dict["inter"], function_thunk_dict))


if __name__ == "__main__":
    main()
