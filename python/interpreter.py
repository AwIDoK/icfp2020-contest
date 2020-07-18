import os
import json
import time
from tkinter import *
import tkinter as tk
import gc
import pickle

import requests
import sys
from PIL import Image, ImageTk

from decoder import decode, decode_to_alien_string
from alien_functions import *
from alien_python import *
from strategy import *

sys.setrecursionlimit(100000)

API_KEY = os.environ['API_KEY']
SEND_URL = "https://icfpc2020-api.testkontur.ru/aliens/send"

start_time = 0

X = 0
Y = 0


def click_processor(event: Event):
    global X, Y
    x = event.x // 4 - 150
    y = event.y // 4 - 150
    X = x
    Y = y
    run_interact(x, y)


window = Tk()
window.geometry("1200x1220")
window.resizable(0, 0)

statusbar = Label(window, bd=1, relief=tk.SUNKEN, anchor=tk.W)
statusbar.pack(side=tk.BOTTOM, fill=tk.X)


def show_coords(event: tk.Event):
    x = event.x // 4 - 150
    y = event.y // 4 - 150
    statusbar.configure(text='{} {}'.format(x, y))

def save():
    global global_state, X, Y
    print("saving")
    state = {
        "x": X,
        "y": Y,
        "state": encode_alien(global_state)
    }
    with open("save.txt", "w") as f:
        json.dump(state, f)


def load():
    global global_state, X, Y
    print("loading")
    with open("save.txt", "r") as f:
        state = json.load(f)
    X = state["x"]
    Y = state["y"]
    global_state = decode_alien(state["state"])[0]
    run_interact(X, Y)

menubar = Menu(window)
filemenu = Menu(menubar, tearoff=0)
filemenu.add_command(label="Save", command=save)
filemenu.add_command(label="Load", command=load)
menubar.add_cascade(label="File", menu=filemenu)
window.config(menu=menubar)

panel = Label(window)
panel.bind('<Button-1>', click_processor)
panel.bind('<Motion>', show_coords)
panel.pack()

global_state = decode_alien('110110010111110110001011010110011001100110011001101111101101000101010101001101101000110000')[0]
prev_state = global_state

global_function_thunk_dict = {}


def send(data):
    return decode_alien(send_encoded(encode_alien(data)))[0]


def send_encoded(encoded_data):
    # print("encoded", encoded_data)
    print('sending', try_describe(decode_to_python(encoded_data)[0]))

    params = {
        "apiKey": API_KEY
    }
    response = requests.post(SEND_URL, params=params, data=encoded_data)
    assert (response.status_code == 200)

    encoded_response = response.content.decode("utf-8")
    # print('received', encoded_response)
    print('received', decode_to_python(encoded_response)[0])

    # print('send time', time.time() - start_time)

    return encoded_response


def f38(protocol_thunk, triple_thunk):
    flag = car(triple_thunk)
    newState = car(cdr(triple_thunk))
    data = car(cdr(cdr(triple_thunk)))

    if extract(flag) == 0:
        multipledraw(extract(data))
        return extract(newState)
    else:
        n_data = send(data)
        return interact(protocol_thunk)(newState)(n_data)


def interact(protocol_thunk):
    def interact1(state_thunk):
        def interact2(data_thunk):
            protocol = extract(protocol_thunk)
            return f38(protocol_thunk, protocol(state_thunk)(data_thunk))
        return interact2

    return interact1


img_id = 0


def draw(points_list):
    global img_id
    img_id += 1
    max_cell = 300
    cell_size = 4
    side = max_cell * cell_size
    img = Image.new('RGB', (side, side))
    pixels = img.load()
    intensity = 255
    while isnil(points_list)(False)(True):
        points = extract(car(points_list))
        points_list = extract(cdr(points_list))

        while isnil(points)(False)(True):
            cur_point = extract(car(points))
            points = extract(cdr(points))
            x, y = extract(car(cur_point)), extract(cdr(cur_point))

            x += 150
            y += 150

            assert (x >= 0 and y >= 0 and x < max_cell and y < max_cell)
            for xi in range(x * cell_size, x * cell_size + cell_size):
                for yi in range(y * cell_size, y * cell_size + cell_size):
                    if pixels[xi, yi] == (0, 0, 0):
                        pixels[xi, yi] = (intensity, intensity, intensity)

        intensity = int(intensity * 0.4)
    # img.save("{}.png".format(img_id), "PNG")
    # img.show()

    pimg = ImageTk.PhotoImage(img)
    panel.configure(image=pimg)
    panel.pack()
    panel.image_ref = pimg


def multipledraw(images):
    draw(images)
    # while isnil(images)(False)(True):
    #     cur_image = extract(car(images))
    #     images = extract(cdr(images))
    #     draw(cur_image)


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

        if callable(lhs) and (callable(rhs) or isinstance(rhs, int)):
            return lhs(rhs), cont2

        return Ap(lhs, rhs), cont2
    elif is_int(tok):
        return int(tok), rem
    elif tok[0] == ":" or tok == 'galaxy':
        if tok in global_function_thunk_dict:
            return global_function_thunk_dict[tok], rem
        return tok, rem
    else:
        if tok in globals():
            return globals()[tok], rem
        print("FAIL", tok)
        exit()
        return (lambda x: tok,), rem


def run_interact(coord1, coord2):
    print('coords', coord1, coord2)

    global global_function_thunk_dict
    function_thunk_dict = global_function_thunk_dict

    global global_state
    prev_state = global_state
    cur_state = global_state
    

    def run(x):
        return interact(evaluate(function_thunk_dict['galaxy'], function_thunk_dict))(cur_state)(cons(coord1)(coord2))

    cur_state = extract(evaluate((run,), function_thunk_dict))
    new_state = encode_alien(cur_state)
    print('new state_enc:', new_state)

    global_state = cur_state

    global memorized
    memorized.clear()
    gc.collect()


def main():
    global start_time
    start_time = time.time()

    file = open('galaxy.txt', 'r')
    global global_function_thunk_dict
    for line in file:
        if line[-1] == "\n":
            line = line[:-1]
        split = line.split("=")
        name = split[0].strip()
        parsed = parse(split[1].strip().split(" "))
        assert(parsed[1] == [])
        global_function_thunk_dict[name] = parsed[0]

    run_interact(0, 0)

    window.title("contest")
    window.mainloop()


if __name__ == "__main__":
    main()

# battle 110110011011110110010111011010001101111111111111111100100000000000110010001001111110101110011101101001111110100111110110101101110000100001101011001100110110010011110110100011001111111101100001110110000111110111000010000011100101011011110100101111011100010000011010110101101100001001101011011100100000011011000010011000011111101011010111101101000011100010010011110110000101100100111101100010110101101100100110110000100110111000111100110111001000000110110000100110000000011110111000010000110101111011110001000000000110110000111011100100000000110011000011001100001101101000110000
# 110110011011110110011011011010001101111111111111111010101110010100011100010011010101001001001100011111000001001111010110111000010001110101100110011011001001111010110011111111011000011101100001111101110001100000101111010010111101111000011010011111010110101101100001001101110010000001101110010000001101100001001100001111110101101011110111000010000010111101001011110101101110010101101101101011110110000100110101101110010000001101100001001100000000111101100100110101111011110001000000000110110000111011100100000000110011000011001100001101101000110000