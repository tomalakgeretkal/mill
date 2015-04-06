set_global("std::always::infix==", make_subroutine<bool, bool>([] (bool a, bool b) {
    return handle(a == b);
}));

set_global("std::always::infix~", make_subroutine<string, string>([] (string const& a, string const& b) {
    return handle(string(a.data() + b.data()));
}));
