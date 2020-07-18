import os
import requests
import sys
from PIL import Image

from decoder import decode
from alien_functions import *
from alien_python import *

sys.setrecursionlimit(100000)

API_KEY = os.environ['API_KEY']
SEND_URL = "https://icfpc2020-api.testkontur.ru/aliens/send"


def send(data):
    encoded_data = encode_alien(data)
    print("encoded", encoded_data)

    params = {
        "apiKey": API_KEY
    }
    response = requests.post(SEND_URL, params=params, data=encoded_data)
    assert(response.status_code == 200)

    encoded_response = response.content.decode("utf-8")
    print(decode(encoded_response)[0])
    alien = decode_alien(encoded_response)
    return alien[0]


def f38(protocol_thunk, triple_thunk):
    flag = car(triple_thunk)
    newState = car(cdr(triple_thunk))
    data = car(cdr(cdr(triple_thunk)))

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


def evaluate(term_thunk, function_thunk_dict):
    term = extract(term_thunk)
    if isinstance(term, str):
        return evaluate(function_thunk_dict[term], function_thunk_dict)
    if isinstance(term, Ap):
        return term.evaluate(function_thunk_dict)
    return term


class Ap:
    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def evaluate(self, function_dict):
        lhs = evaluate(self.lhs, function_dict)
        rhs_thunk = self.rhs
        eval_thunk = (lambda x: evaluate(rhs_thunk, function_dict)),
        return lhs(eval_thunk)


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
