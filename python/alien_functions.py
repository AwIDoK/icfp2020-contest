memorized = {}

def extract(x):
    if x in memorized:
        return memorized[x]
    if isinstance(x, tuple):
        result = extract(x[0](())) # should work without extract
    else:
        result = x
    memorized[x] = result
    return result

# 18
def s(x_thunk):
    def s1(y_thunk):
        def s2(z_thunk):
            x = extract(x_thunk)
            def ap(x):
                y = extract(y_thunk)
                return y(z_thunk)
            ap_thunk = (ap,)
            return extract(x(z_thunk))(ap_thunk)
        return s2
    return s1


# 19
def c(x_thunk):
    def c1(y_thunk):
        def c2(z_thunk):
            x = extract(x_thunk)
            return extract(x(z_thunk))(y_thunk)
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
            return extract(extract(foo_thunk)(x_thunk))(y_thunk)
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
