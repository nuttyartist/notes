/*
 * Copyright (c) 2019 Waqar Ahmed -- <waqar.17a@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 */
#include <QMultiHash>

/* ------------------------
 * TEMPLATE FOR LANG DATA
 * -------------------------
 *
 * loadXXXData, where XXX is the language
 * keywords are the language keywords e.g, const
 * types are built-in types i.e, int, char, var
 * literals are words like, true false
 * builtin are the library functions
 * other can contain any other thing, for e.g, in cpp it contains the preprocessor

    static const QMultiHash<char, QLatin1String> xxx_keywords = {
    };

    static const QMultiHash<char, QLatin1String> xxx_types = {
    };

    static const QMultiHash<char, QLatin1String> xxx_literals = {
    };

    static const QMultiHash<char, QLatin1String> xxx_builtin = {
    };

    static const QMultiHash<char, QLatin1String> xxx_other = {
    };

*/

/**********************************************************/
/* C/C++ Data *********************************************/
/**********************************************************/

static const QMultiHash<char, QLatin1String> cpp_keywords = {
    {('a'), QLatin1String("asm")},
    {('a'), QLatin1String("auto")},
    {('b'), QLatin1String("break")},
    {('c'), QLatin1String("case")},
    {('c'), QLatin1String("catch")},
    {('c'), QLatin1String("const")},
    {('c'), QLatin1String("const_cast")},
    {('c'), QLatin1String("continue")},
    {('d'), QLatin1String("default")},
    {('d'), QLatin1String("delete")},
    {('d'), QLatin1String("do")},
    {('d'), QLatin1String("dynamic_cast")},
    {('e'), QLatin1String("else")},
    {('e'), QLatin1String("explicit")},
    {('f'), QLatin1String("for")},
    {('g'), QLatin1String("goto")},
    {('i'), QLatin1String("if")},
    {('m'), QLatin1String("mutable")},
    {('n'), QLatin1String("namespace")},
    {('n'), QLatin1String("new")},
    {('o'), QLatin1String("operator")},
    {('p'), QLatin1String("private")},
    {('p'), QLatin1String("protected")},
    {('p'), QLatin1String("public")},
    {('r'), QLatin1String("register")},
    {('r'), QLatin1String("reinterpret_cast")},
    {('r'), QLatin1String("return")},
    {('s'), QLatin1String("signal")},
    {('s'), QLatin1String("signed")},
    {('s'), QLatin1String("sizeof")},
    {('s'), QLatin1String("slot")},
    {('s'), QLatin1String("static")},
    {('s'), QLatin1String("static_cast")},
    {('s'), QLatin1String("switch")},
    {('t'), QLatin1String("template")},
    {('t'), QLatin1String("this")},
    {('t'), QLatin1String("throw")},
    {('t'), QLatin1String("try")},
    {('t'), QLatin1String("typedef")},
    {('u'), QLatin1String("unsigned")},
    {('u'), QLatin1String("using")},
    {('v'), QLatin1String("volatile")},
    {('w'), QLatin1String("while")}
};

static const QMultiHash<char, QLatin1String> cpp_types = {
    {('b'), QLatin1String("bool")},
    {('c'), QLatin1String("char")},
    {('c'), QLatin1String("class")},
    {('d'), QLatin1String("double")},
    {('e'), QLatin1String("enum")},
    {('f'), QLatin1String("float")},
    {('i'), QLatin1String("int")},
    {('l'), QLatin1String("long")},
    {('Q'), QLatin1String("QHash")},
    {('Q'), QLatin1String("QList")},
    {('Q'), QLatin1String("QMap")},
    {('Q'), QLatin1String("QString")},
    {('Q'), QLatin1String("QVector")},
    {('s'), QLatin1String("short")},
    {('s'), QLatin1String("size_t")},
    {('s'), QLatin1String("ssize_t")},
    {('s'), QLatin1String("struct")},
    {('u'), QLatin1String("union")},
    {('u'), QLatin1String("uint8_t")},
    {('u'), QLatin1String("uint16_t")},
    {('u'), QLatin1String("uint32_t")},
    {('u'), QLatin1String("uint64_t")},
    {('v'), QLatin1String("void")},
    {('w'), QLatin1String("wchar_t")}
};

static const QMultiHash<char, QLatin1String> cpp_literals = {
    {('f'), QLatin1String("false")},
    {('n'), QLatin1String("nullptr")},
    {('N'), QLatin1String("NULL")},
    {('t'), QLatin1String("true")}
};

static const QMultiHash<char, QLatin1String> cpp_builtin = {
    {('s'), QLatin1String("std")},
    {('s'), QLatin1String("string")},
    {('w'), QLatin1String("wstring")},
    {('c'), QLatin1String("cin")},
    {('c'), QLatin1String("cout")},
    {('c'), QLatin1String("cerr")},
    {('c'), QLatin1String("clog")},
    {('s'), QLatin1String("stdin")},
    {('s'), QLatin1String("stdout")},
    {('s'), QLatin1String("stderr")},
    {('s'), QLatin1String("stringstream")},
    {('i'), QLatin1String("istringstream")},
    {('o'), QLatin1String("ostringstream")},
    {('a'), QLatin1String("auto_ptr")},
    {('d'), QLatin1String("deque")},
    {('l'), QLatin1String("list")},
    {('q'), QLatin1String("queue")},
    {('s'), QLatin1String("stack")},
    {('v'), QLatin1String("vector")},
    {('m'), QLatin1String("map")},
    {('s'), QLatin1String("set")},
    {('b'), QLatin1String("bitset")},
    {('m'), QLatin1String("multiset")},
    {('m'), QLatin1String("multimap")},
    {('u'), QLatin1String("unordered_set")},
    {('u'), QLatin1String("unordered_map")},
    {('u'), QLatin1String("unordered_multiset")},
    {('u'), QLatin1String("unordered_multimap")},
    {('a'), QLatin1String("array")},
    {('s'), QLatin1String("shared_ptr")},
    {('a'), QLatin1String("abort")},
    {('t'), QLatin1String("terminate")},
    {('a'), QLatin1String("abs")},
    {('a'), QLatin1String("acos")},
    {('a'), QLatin1String("asin")},
    {('a'), QLatin1String("atan2")},
    {('a'), QLatin1String("atan")},
    {('c'), QLatin1String("calloc")},
    {('c'), QLatin1String("ceil")},
    {('c'), QLatin1String("cosh")},
    {('c'), QLatin1String("cos")},
    {('e'), QLatin1String("exit")},
    {('e'), QLatin1String("exp")},
    {('f'), QLatin1String("fabs")},
    {('f'), QLatin1String("floor")},
    {('f'), QLatin1String("fmod")},
    {('f'), QLatin1String("fprintf")},
    {('f'), QLatin1String("fputs")},
    {('f'), QLatin1String("free")},
    {('f'), QLatin1String("frexp")},
    {('f'), QLatin1String("fscanf")},
    {('f'), QLatin1String("future")},
    {('i'), QLatin1String("isalnum")},
    {('i'), QLatin1String("isalpha")},
    {('i'), QLatin1String("iscntrl")},
    {('i'), QLatin1String("isdigit")},
    {('i'), QLatin1String("isgraph")},
    {('i'), QLatin1String("islower")},
    {('i'), QLatin1String("isprint")},
    {('i'), QLatin1String("ispunct")},
    {('i'), QLatin1String("isspace")},
    {('i'), QLatin1String("isupper")},
    {('i'), QLatin1String("isxdigit")},
    {('t'), QLatin1String("tolower")},
    {('t'), QLatin1String("toupper")},
    {('l'), QLatin1String("labs")},
    {('l'), QLatin1String("ldexp")},
    {('l'), QLatin1String("log10")},
    {('l'), QLatin1String("log")},
    {('m'), QLatin1String("malloc")},
    {('r'), QLatin1String("realloc")},
    {('m'), QLatin1String("main")},
    {('m'), QLatin1String("memchr")},
    {('m'), QLatin1String("memcmp")},
    {('m'), QLatin1String("memcpy")},
    {('m'), QLatin1String("memset")},
    {('m'), QLatin1String("modf")},
    {('p'), QLatin1String("pow")},
    {('p'), QLatin1String("printf")},
    {('p'), QLatin1String("putchar")},
    {('p'), QLatin1String("puts")},
    {('s'), QLatin1String("scanf")},
    {('s'), QLatin1String("sinh")},
    {('s'), QLatin1String("sin")},
    {('s'), QLatin1String("snprintf")},
    {('s'), QLatin1String("sprintf")},
    {('s'), QLatin1String("sqrt")},
    {('s'), QLatin1String("sscanf")},
    {('s'), QLatin1String("strcat")},
    {('s'), QLatin1String("strchr")},
    {('s'), QLatin1String("strcmp")},
    {('s'), QLatin1String("strcpy")},
    {('s'), QLatin1String("strcspn")},
    {('s'), QLatin1String("strlen")},
    {('s'), QLatin1String("strncat")},
    {('s'), QLatin1String("strncmp")},
    {('s'), QLatin1String("strncpy")},
    {('s'), QLatin1String("strpbrk")},
    {('s'), QLatin1String("strrchr")},
    {('s'), QLatin1String("strspn")},
    {('s'), QLatin1String("strstr")},
    {('t'), QLatin1String("tanh")},
    {('t'), QLatin1String("tan")},
    {('v'), QLatin1String("vfprintf")},
    {('v'), QLatin1String("vprintf")},
    {('v'), QLatin1String("vsprintf")},
    {('e'), QLatin1String("endl")},
    {('i'), QLatin1String("initializer_list")},
    {('u'), QLatin1String("unique_ptr")},
    {('c'), QLatin1String("complex")},
    {('i'), QLatin1String("imaginary")}
};

static const QMultiHash<char, QLatin1String> cpp_other = {
    {('d'), QLatin1String("define")},
    {('e'), QLatin1String("else")},
    {('e'), QLatin1String("elif")},
    {('e'), QLatin1String("endif")},
    {('e'), QLatin1String("error")},
    {('i'), QLatin1String("if")},
    {('i'), QLatin1String("ifdef")},
    {('i'), QLatin1String("ifndef")},
    {('i'), QLatin1String("include")},
    {('l'), QLatin1String("line")},
    {('p'), QLatin1String("pragma")},
    {('P'), QLatin1String("_Pragma")},
    {('u'), QLatin1String("undef")},
    {('w'), QLatin1String("warning")}
};

void loadCppData(QMultiHash<char, QLatin1String> &typess,
             QMultiHash<char, QLatin1String> &keywordss,
             QMultiHash<char, QLatin1String> &builtins,
             QMultiHash<char, QLatin1String> &literalss,
             QMultiHash<char, QLatin1String> &others){
    typess = cpp_types;
    keywordss = cpp_keywords;
    builtins = cpp_builtin;
    literalss = cpp_literals;
    others = cpp_other;

}

/**********************************************************/
/* Shell Data *********************************************/
/**********************************************************/

static const QMultiHash<char, QLatin1String> shell_keywords = {
    {('i'), QLatin1String("if")},
    {('t'), QLatin1String("then")},
    {('e'), QLatin1String("else")},
    {('e'), QLatin1String("elif")},
    {('f'), QLatin1String("fi")},
    {('f'), QLatin1String("for")},
    {('w'), QLatin1String("while")},
    {('i'), QLatin1String("in")},
    {('d'), QLatin1String("do")},
    {('d'), QLatin1String("done")},
    {('c'), QLatin1String("case")},
    {('e'), QLatin1String("esac")},
    {('f'), QLatin1String("function")}
};

static const QMultiHash<char, QLatin1String> shell_types = {
};

static const QMultiHash<char, QLatin1String> shell_literals = {
    {('f'), QLatin1String("false")},
    {('t'), QLatin1String("true")}
};

static const QMultiHash<char, QLatin1String> shell_builtin = {
    {('b'), QLatin1String("break")},
    {('c'), QLatin1String("cd")},
    {('c'), QLatin1String("continue")},
    {('e'), QLatin1String("eval")},
    {('e'), QLatin1String("exec")},
    {('e'), QLatin1String("exit")},
    {('e'), QLatin1String("export")},
    {('g'), QLatin1String("getopts")},
    {('h'), QLatin1String("hash")},
    {('p'), QLatin1String("pwd")},
    {('r'), QLatin1String("readonly")},
    {('r'), QLatin1String("return")},
    {('s'), QLatin1String("shift")},
    {('t'), QLatin1String("test")},
    {('t'), QLatin1String("timestrap")},
    {('u'), QLatin1String("umask")},
    {('u'), QLatin1String("unset")},
    {('B'), QLatin1String("Bash")},
    {('a'), QLatin1String("alias")},
    {('b'), QLatin1String("bind")},
    {('b'), QLatin1String("builtin")},
    {('c'), QLatin1String("caller")},
    {('c'), QLatin1String("command")},
    {('d'), QLatin1String("declare")},
    {('e'), QLatin1String("echo")},
    {('e'), QLatin1String("enable")},
    {('h'), QLatin1String("help")},
    {('l'), QLatin1String("let")},
    {('l'), QLatin1String("local")},
    {('l'), QLatin1String("logout")},
    {('m'), QLatin1String("mapfile")},
    {('p'), QLatin1String("printfread")},
    {('r'), QLatin1String("readarray")},
    {('s'), QLatin1String("source")},
    {('t'), QLatin1String("type")},
    {('t'), QLatin1String("typeset")},
    {('u'), QLatin1String("ulimit")},
    {('u'), QLatin1String("unalias")},
    {('m'), QLatin1String("modifiers")},
    {('s'), QLatin1String("set")},
    {('s'), QLatin1String("shopt")},
    {('a'), QLatin1String("autoload")},
    {('b'), QLatin1String("bg")},
    {('b'), QLatin1String("bindkey")},
    {('b'), QLatin1String("bye")},
    {('c'), QLatin1String("cap")},
    {('c'), QLatin1String("chdir")},
    {('c'), QLatin1String("clone")},
    {('c'), QLatin1String("comparguments")},
    {('c'), QLatin1String("compcall")},
    {('c'), QLatin1String("compctl")},
    {('c'), QLatin1String("compdescribe")},
    {('c'), QLatin1String("compfilescompgroups")},
    {('c'), QLatin1String("compquote")},
    {('c'), QLatin1String("comptags")},
    {('c'), QLatin1String("comptry")},
    {('c'), QLatin1String("compvalues")},
    {('d'), QLatin1String("dirs")},
    {('d'), QLatin1String("disable")},
    {('d'), QLatin1String("disown")},
    {('e'), QLatin1String("echotc")},
    {('e'), QLatin1String("echoti")},
    {('e'), QLatin1String("emulatefc")},
    {('f'), QLatin1String("fg")},
    {('f'), QLatin1String("float")},
    {('f'), QLatin1String("functions")},
    {('g'), QLatin1String("getcap")},
    {('g'), QLatin1String("getln")},
    {('h'), QLatin1String("history")},
    {('i'), QLatin1String("integer")},
    {('j'), QLatin1String("jobs")},
    {('k'), QLatin1String("kill")},
    {('l'), QLatin1String("limit")},
    {('l'), QLatin1String("log")},
    {('n'), QLatin1String("noglob")},
    {('p'), QLatin1String("popd")},
    {('p'), QLatin1String("printpushd")},
    {('p'), QLatin1String("pushln")},
    {('r'), QLatin1String("rehash")},
    {('s'), QLatin1String("sched")},
    {('s'), QLatin1String("setcap")},
    {('s'), QLatin1String("setopt")},
    {('s'), QLatin1String("stat")},
    {('s'), QLatin1String("suspend")},
    {('t'), QLatin1String("ttyctl")},
    {('u'), QLatin1String("unfunction")},
    {('u'), QLatin1String("unhash")},
    {('u'), QLatin1String("unlimitunsetopt")},
    {('v'), QLatin1String("vared")},
    {('w'), QLatin1String("wait")},
    {('w'), QLatin1String("whence")},
    {('w'), QLatin1String("where")},
    {('w'), QLatin1String("which")},
    {('z'), QLatin1String("zcompile")},
    {('z'), QLatin1String("zformat")},
    {('z'), QLatin1String("zftp")},
    {('z'), QLatin1String("zle")},
    {('z'), QLatin1String("zmodload")},
    {('z'), QLatin1String("zparseopts")},
    {('z'), QLatin1String("zprof")},
    {('z'), QLatin1String("zpty")},
    {('z'), QLatin1String("zregexparse")},
    {('z'), QLatin1String("zsocket")},
    {('z'), QLatin1String("zstyle")},
    {('z'), QLatin1String("ztcp")}
};

static const QMultiHash<char, QLatin1String> shell_other = {
};

void loadShellData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other){
    types = shell_types;
    keywords = shell_keywords;
    builtin = shell_builtin;
    literals = shell_literals;
    other = shell_other;

}

/**********************************************************/
/* JS Data *********************************************/
/**********************************************************/

static const QMultiHash<char, QLatin1String> js_keywords = {
    {('i'), QLatin1String("in")},
    {('o'), QLatin1String("of")},
    {('i'), QLatin1String("if")},
    {('f'), QLatin1String("for")},
    {('w'), QLatin1String("while")},
    {('f'), QLatin1String("finally")},
    {('n'), QLatin1String("new")},
    {('f'), QLatin1String("function")},
    {('d'), QLatin1String("do")},
    {('r'), QLatin1String("return")},
    {('v'), QLatin1String("void")},
    {('e'), QLatin1String("else")},
    {('b'), QLatin1String("break")},
    {('c'), QLatin1String("catch")},
    {('i'), QLatin1String("instanceof")},
    {('w'), QLatin1String("with")},
    {('t'), QLatin1String("throw")},
    {('c'), QLatin1String("case")},
    {('d'), QLatin1String("default")},
    {('t'), QLatin1String("try")},
    {('t'), QLatin1String("this")},
    {('s'), QLatin1String("switch")},
    {('c'), QLatin1String("continue")},
    {('t'), QLatin1String("typeof")},
    {('d'), QLatin1String("delete")},
    {('l'), QLatin1String("let")},
    {('y'), QLatin1String("yield")},
    {('c'), QLatin1String("const")},
    {('e'), QLatin1String("export")},
    {('s'), QLatin1String("super")},
    {('d'), QLatin1String("debugger")},
    {('a'), QLatin1String("as")},
    {('a'), QLatin1String("async")},
    {('a'), QLatin1String("await")},
    {('s'), QLatin1String("static")},
    {('i'), QLatin1String("import")},
    {('f'), QLatin1String("from")},
    {('a'), QLatin1String("as")}
};

static const QMultiHash<char, QLatin1String> js_types = {
    {('v'), QLatin1String("var")},
    {('c'), QLatin1String("class")},
    {('b'), QLatin1String("byte")},
    {('e'), QLatin1String("enum")},
    {('f'), QLatin1String("float")},
    {('s'), QLatin1String("short")},
    {('l'), QLatin1String("long")},
    {('i'), QLatin1String("int")},
    {('v'), QLatin1String("void")},
    {('b'), QLatin1String("boolean")},
    {('d'), QLatin1String("double")}
};

static const QMultiHash<char, QLatin1String> js_literals = {
    {('f'), QLatin1String("false")},
    {('n'), QLatin1String("null")},
    {('t'), QLatin1String("true")},
    {('u'), QLatin1String("undefined")},
    {('N'), QLatin1String("NaN")},
    {('I'), QLatin1String("Infinity")}
};

static const QMultiHash<char, QLatin1String> js_builtin = {
    {('e'), QLatin1String("eval")},
    {('i'), QLatin1String("isFinite")},
    {('i'), QLatin1String("isNaN")},
    {('p'), QLatin1String("parseFloat")},
    {('p'), QLatin1String("parseInt")},
    {('d'), QLatin1String("decodeURI")},
    {('d'), QLatin1String("decodeURIComponent")},
    {('e'), QLatin1String("encodeURI")},
    {('e'), QLatin1String("encodeURIComponent")},
    {('e'), QLatin1String("escape")},
    {('u'), QLatin1String("unescape")},
    {('O'), QLatin1String("Object")},
    {('F'), QLatin1String("Function")},
    {('B'), QLatin1String("Boolean")},
    {('E'), QLatin1String("Error")},
    {('E'), QLatin1String("EvalError")},
    {('I'), QLatin1String("InternalError")},
    {('R'), QLatin1String("RangeError")},
    {('R'), QLatin1String("ReferenceError")},
    {('S'), QLatin1String("StopIteration")},
    {('S'), QLatin1String("SyntaxError")},
    {('T'), QLatin1String("TypeError")},
    {('U'), QLatin1String("URIError")},
    {('N'), QLatin1String("Number")},
    {('M'), QLatin1String("Math")},
    {('D'), QLatin1String("Date")},
    {('S'), QLatin1String("String")},
    {('R'), QLatin1String("RegExp")},
    {('A'), QLatin1String("Array")},
    {('F'), QLatin1String("Float32Array")},
    {('F'), QLatin1String("Float64Array")},
    {('I'), QLatin1String("Int16Array")},
    {('I'), QLatin1String("Int32Array")},
    {('I'), QLatin1String("Int8Array")},
    {('U'), QLatin1String("Uint16Array")},
    {('U'), QLatin1String("Uint32Array")},
    {('U'), QLatin1String("Uint8Array")},
    {('U'), QLatin1String("Uint8ClampedArray")},
    {('A'), QLatin1String("ArrayBuffer")},
    {('D'), QLatin1String("DataView")},
    {('J'), QLatin1String("JSON")},
    {('I'), QLatin1String("Intl")},
    {('a'), QLatin1String("arguments")},
    {('r'), QLatin1String("require")},
    {('m'), QLatin1String("module")},
    {('c'), QLatin1String("console")},
    {('w'), QLatin1String("window")},
    {('d'), QLatin1String("document")},
    {('S'), QLatin1String("Symbol")},
    {('S'), QLatin1String("Set")},
    {('M'), QLatin1String("Map")},
    {('W'), QLatin1String("WeakSet")},
    {('W'), QLatin1String("WeakMap")},
    {('P'), QLatin1String("Proxy")},
    {('R'), QLatin1String("Reflect")},
    {('P'), QLatin1String("Promise")}
};

static const QMultiHash<char, QLatin1String> js_other = {
};

void loadJSData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other){
    types = js_types;
    keywords = js_keywords;
    builtin = js_builtin;
    literals = js_literals;
    other = js_other;
}

/**********************************************************/
/* PHP Data *********************************************/
/**********************************************************/

static const QMultiHash<char, QLatin1String> php_keywords = {
    {('a'), QLatin1String("and")},
    {('l'), QLatin1String("list")},
    {('a'), QLatin1String("abstract")},
    {('g'), QLatin1String("global")},
    {('p'), QLatin1String("private")},
    {('e'), QLatin1String("echo")},
    {('i'), QLatin1String("interface")},
    {('a'), QLatin1String("as")},
    {('s'), QLatin1String("static")},
    {('e'), QLatin1String("endswitch")},
    {('i'), QLatin1String("if")},
    {('e'), QLatin1String("endwhile")},
    {('o'), QLatin1String("or")},
    {('c'), QLatin1String("const")},
    {('f'), QLatin1String("for")},
    {('e'), QLatin1String("endforeach")},
    {('s'), QLatin1String("self")},
    {('w'), QLatin1String("while")},
    {('i'), QLatin1String("isset")},
    {('p'), QLatin1String("public")},
    {('p'), QLatin1String("protected")},
    {('e'), QLatin1String("exit")},
    {('f'), QLatin1String("foreach")},
    {('t'), QLatin1String("throw")},
    {('e'), QLatin1String("elseif")},
    {('e'), QLatin1String("empty")},
    {('d'), QLatin1String("do")},
    {('x'), QLatin1String("xor")},
    {('r'), QLatin1String("return")},
    {('p'), QLatin1String("parent")},
    {('c'), QLatin1String("clone")},
    {('u'), QLatin1String("use")},
    {('e'), QLatin1String("else")},
    {('b'), QLatin1String("break")},
    {('p'), QLatin1String("print")},
    {('e'), QLatin1String("eval")},
    {('n'), QLatin1String("new")},
    {('c'), QLatin1String("catch")},
    {('c'), QLatin1String("case")},
    {('e'), QLatin1String("exception")},
    {('d'), QLatin1String("default")},
    {('d'), QLatin1String("die")},
    {('e'), QLatin1String("enddeclare")},
    {('f'), QLatin1String("final")},
    {('t'), QLatin1String("try")},
    {('s'), QLatin1String("switch")},
    {('c'), QLatin1String("continue")},
    {('e'), QLatin1String("endfor")},
    {('e'), QLatin1String("endif")},
    {('d'), QLatin1String("declare")},
    {('u'), QLatin1String("unset")},
    {('t'), QLatin1String("trait")},
    {('g'), QLatin1String("goto")},
    {('i'), QLatin1String("instanceof")},
    {('i'), QLatin1String("insteadof")},
    {('y'), QLatin1String("yield")},
    {('f'), QLatin1String("finally")}
};

static const QMultiHash<char, QLatin1String> php_types = {
    {('v'), QLatin1String("var")},
    {('c'), QLatin1String("class")},
    {('e'), QLatin1String("enum")},
    {('a'), QLatin1String("array")}
};

static const QMultiHash<char, QLatin1String> php_literals = {
    {('f'), QLatin1String("false")},
    {('t'), QLatin1String("true")},
    {('n'), QLatin1String("null")}
};

static const QMultiHash<char, QLatin1String> php_builtin = {


};

static const QMultiHash<char, QLatin1String> php_other = {
    {('i'), QLatin1String("include_once")},
    {('i'), QLatin1String("include")},
    {('_'), QLatin1String("__FILE__")},
    {('r'), QLatin1String("require")},
    {('r'), QLatin1String("require_once")},
    {('_'), QLatin1String("__CLASS__")},
    {('_'), QLatin1String("__LINE__")},
    {('_'), QLatin1String("__METHOD__")},
    {('_'), QLatin1String("__FUNCTION__")},
    {('_'), QLatin1String("__DIR__")},
    {('_'), QLatin1String("__NAMESPACE__")},

    {('S'), QLatin1String("SERVER")},
    {('G'), QLatin1String("GET")},
    {('P'), QLatin1String("POST")},
    {('F'), QLatin1String("FILES")},
    {('R'), QLatin1String("REQUEST")},
    {('S'), QLatin1String("SESSION")},
    {('E'), QLatin1String("ENV")},
    {('C'), QLatin1String("COOKIE")},
    {('G'), QLatin1String("GLOBALS")},
    {('H'), QLatin1String("HTTP_RAW_POST_DATA")},
    {('a'), QLatin1String("argc")},
    {('a'), QLatin1String("argv")},
    {('p'), QLatin1String("php_errormsg")},
    {('h'), QLatin1String("http_response_header")}
};

void loadPHPData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other){
    types = php_types;
    keywords = php_keywords;
    builtin = php_builtin;
    literals = php_literals;
    other = php_other;
}

/**********************************************************/
/* QML Data *********************************************/
/**********************************************************/

static const QMultiHash<char, QLatin1String> qml_keywords = {
    {('d'), QLatin1String("default")},
    {('p'), QLatin1String("property")},
    {('i'), QLatin1String("int")},
    {('v'), QLatin1String("var")},
    {('s'), QLatin1String("string")},
    {('f'), QLatin1String("function")},
    {('r'), QLatin1String("readonly")},
    {('M'), QLatin1String("MouseArea")},
    {('d'), QLatin1String("delegate")},
    {('i'), QLatin1String("if")},
    {('e'), QLatin1String("else")},

    {('e'), QLatin1String("eval")},
    {('i'), QLatin1String("isFinite")},
    {('i'), QLatin1String("isNaN")},
    {('p'), QLatin1String("parseFloat")},
    {('p'), QLatin1String("parseInt")},
    {('d'), QLatin1String("decodeURI")},
    {('d'), QLatin1String("decodeURIComponent")},
    {('e'), QLatin1String("encodeURI")},
    {('e'), QLatin1String("encodeURIComponent")},
    {('e'), QLatin1String("escape")},
    {('u'), QLatin1String("unescape")},
    {('O'), QLatin1String("Object")},
    {('E'), QLatin1String("Error")},
    {('E'), QLatin1String("EvalError")},
    {('I'), QLatin1String("InternalError")},
    {('R'), QLatin1String("RangeError")},
    {('R'), QLatin1String("ReferenceError")},
    {('S'), QLatin1String("StopIteration")},
    {('S'), QLatin1String("SyntaxError")},
    {('T'), QLatin1String("TypeError")},
    {('U'), QLatin1String("URIError")},
    {('N'), QLatin1String("Number")},
    {('M'), QLatin1String("Math")},
    {('D'), QLatin1String("Date")},
    {('S'), QLatin1String("String")},
    {('R'), QLatin1String("RegExp")},
    {('A'), QLatin1String("Array")},
    {('F'), QLatin1String("Float32Array")},
    {('F'), QLatin1String("Float64Array")},
    {('I'), QLatin1String("Int16Array")},
    {('I'), QLatin1String("Int32Array")},
    {('I'), QLatin1String("Int8Array")},
    {('U'), QLatin1String("Uint16Array")},
    {('U'), QLatin1String("Uint32Array")},
    {('U'), QLatin1String("Uint8Array")},
    {('U'), QLatin1String("Uint8ClampedArray")},
    {('A'), QLatin1String("ArrayBuffer")},
    {('D'), QLatin1String("DataView")},
    {('J'), QLatin1String("JSON")},
    {('I'), QLatin1String("Intl")},
    {('a'), QLatin1String("arguments")},
    {('m'), QLatin1String("module")},
    {('c'), QLatin1String("console")},
    {('w'), QLatin1String("window")},
    {('d'), QLatin1String("document")},
    {('S'), QLatin1String("Symbol")},
    {('S'), QLatin1String("Set")},
    {('M'), QLatin1String("Map")},
    {('W'), QLatin1String("WeakSet")},
    {('W'), QLatin1String("WeakMap")},
    {('P'), QLatin1String("Proxy")},
    {('R'), QLatin1String("Reflect")},
    {('B'), QLatin1String("Behavior")},
    {('c'), QLatin1String("color")},
    {('c'), QLatin1String("coordinate")},
    {('d'), QLatin1String("date")},
    {('e'), QLatin1String("enumeration")},
    {('f'), QLatin1String("font")},
    {('g'), QLatin1String("geocircle")},
    {('g'), QLatin1String("georectangle")},
    {('g'), QLatin1String("geoshape")},
    {('l'), QLatin1String("list")},
    {('m'), QLatin1String("matrix4x4")},
    {('p'), QLatin1String("parent")},
    {('p'), QLatin1String("point")},
    {('q'), QLatin1String("quaternion")},
    {('r'), QLatin1String("real")},
    {('s'), QLatin1String("size")},
    {('s'), QLatin1String("string")},
    {('v'), QLatin1String("variant")},
    {('v'), QLatin1String("vector2d")},
    {('v'), QLatin1String("vector3d")},
    {('v'), QLatin1String("vector4d")},
    {('P'), QLatin1String("Promise")}
};

static const QMultiHash<char, QLatin1String> qml_types = {
    {('R'), QLatin1String("Rectangle")},
    {('T'), QLatin1String("Text")},
    {('c'), QLatin1String("color")},
    {('I'), QLatin1String("Item")},
    {('u'), QLatin1String("url")},
    {('C'), QLatin1String("Component")},
    {('B'), QLatin1String("Button")},
    {('T'), QLatin1String("TextInput")},
    {('L'), QLatin1String("ListView")},


};

static const QMultiHash<char, QLatin1String> qml_literals = {
    {('f'), QLatin1String("false")},
    {('t'), QLatin1String("true")}
};

static const QMultiHash<char, QLatin1String> qml_builtin = {

};

static const QMultiHash<char, QLatin1String> qml_other = {
    {('i'), QLatin1String("import")}
};

void loadQMLData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other){
    types = qml_types;
    keywords = qml_keywords;
    builtin = qml_builtin;
    literals = qml_literals;
    other = qml_other;
}

/**********************************************************/
/* Python Data *********************************************/
/**********************************************************/

static const QMultiHash<char, QLatin1String> py_keywords = {
    {('a'), QLatin1String("and")},
    {('e'), QLatin1String("elif")},
    {('i'), QLatin1String("is")},
    {('g'), QLatin1String("global")},
    {('a'), QLatin1String("as")},
    {('i'), QLatin1String("in")},
    {('i'), QLatin1String("if")},
    {('f'), QLatin1String("from")},
    {('r'), QLatin1String("raise")},
    {('f'), QLatin1String("for")},
    {('e'), QLatin1String("except")},
    {('f'), QLatin1String("finally")},
    {('p'), QLatin1String("print")},
    {('p'), QLatin1String("pass")},
    {('r'), QLatin1String("return")},
    {('e'), QLatin1String("exec")},
    {('e'), QLatin1String("else")},
    {('b'), QLatin1String("break")},
    {('n'), QLatin1String("not")},
    {('w'), QLatin1String("with")},
    {('c'), QLatin1String("class")},
    {('a'), QLatin1String("assert")},
    {('y'), QLatin1String("yield")},
    {('t'), QLatin1String("try")},
    {('w'), QLatin1String("while")},
    {('c'), QLatin1String("continue")},
    {('d'), QLatin1String("del")},
    {('o'), QLatin1String("or")},
    {('d'), QLatin1String("def")},
    {('l'), QLatin1String("lambda")},
    {('a'), QLatin1String("async")},
    {('a'), QLatin1String("await")},
    {('n'), QLatin1String("nonlocal")},
};

static const QMultiHash<char, QLatin1String> py_types = {

};

static const QMultiHash<char, QLatin1String> py_literals = {
    {('F'), QLatin1String("False")},
    {('T'), QLatin1String("True")},
    {('N'), QLatin1String("None")}
};

static const QMultiHash<char, QLatin1String> py_builtin = {
    {('_'), QLatin1String("__import__")},
    {('a'), QLatin1String("abs")},
    {('a'), QLatin1String("all")},
    {('a'), QLatin1String("any")},
    {('a'), QLatin1String("apply")},
    {('a'), QLatin1String("ascii")},
    {('b'), QLatin1String("basestring")},
    {('b'), QLatin1String("bin")},
    {('b'), QLatin1String("bool")},
    {('b'), QLatin1String("buffer")},
    {('b'), QLatin1String("bytearray")},
    {('b'), QLatin1String("bytes")},
    {('c'), QLatin1String("callable")},
    {('c'), QLatin1String("chr")},
    {('c'), QLatin1String("classmethod")},
    {('c'), QLatin1String("cmp")},
    {('c'), QLatin1String("coerce")},
    {('c'), QLatin1String("compile")},
    {('c'), QLatin1String("complex")},
    {('d'), QLatin1String("delattr")},
    {('d'), QLatin1String("dict")},
    {('d'), QLatin1String("dir")},
    {('d'), QLatin1String("divmod")},
    {('e'), QLatin1String("enumerate")},
    {('e'), QLatin1String("eval")},
    {('e'), QLatin1String("execfile")},
    {('f'), QLatin1String("file")},
    {('f'), QLatin1String("filter")},
    {('f'), QLatin1String("float")},
    {('f'), QLatin1String("format")},
    {('f'), QLatin1String("frozenset")},
    {('g'), QLatin1String("getattr")},
    {('g'), QLatin1String("globals")},
    {('h'), QLatin1String("hasattr")},
    {('h'), QLatin1String("hash")},
    {('h'), QLatin1String("help")},
    {('h'), QLatin1String("hex")},
    {('i'), QLatin1String("id")},
    {('i'), QLatin1String("input")},
    {('i'), QLatin1String("int")},
    {('i'), QLatin1String("intern")},
    {('i'), QLatin1String("isinstance")},
    {('i'), QLatin1String("issubclass")},
    {('i'), QLatin1String("iter")},
    {('l'), QLatin1String("len")},
    {('l'), QLatin1String("list")},
    {('l'), QLatin1String("locals")},
    {('l'), QLatin1String("long")},
    {('m'), QLatin1String("map")},
    {('m'), QLatin1String("max")},
    {('m'), QLatin1String("memoryview")},
    {('m'), QLatin1String("min")},
    {('n'), QLatin1String("next")},
    {('o'), QLatin1String("object")},
    {('o'), QLatin1String("oct")},
    {('o'), QLatin1String("open")},
    {('o'), QLatin1String("ord")},
    {('p'), QLatin1String("pow")},
    {('p'), QLatin1String("property")},
    {('r'), QLatin1String("range")},
    {('r'), QLatin1String("raw_input")},
    {('r'), QLatin1String("reduce")},
    {('r'), QLatin1String("reload")},
    {('r'), QLatin1String("repr")},
    {('r'), QLatin1String("reversed")},
    {('r'), QLatin1String("round")},
    {('s'), QLatin1String("set")},
    {('s'), QLatin1String("setattr")},
    {('s'), QLatin1String("slice")},
    {('s'), QLatin1String("sorted")},
    {('s'), QLatin1String("staticmethod")},
    {('s'), QLatin1String("str")},
    {('s'), QLatin1String("sum")},
    {('s'), QLatin1String("super")},
    {('t'), QLatin1String("tuple")},
    {('t'), QLatin1String("type")},
    {('u'), QLatin1String("unichr")},
    {('u'), QLatin1String("unicode")},
    {('v'), QLatin1String("vars")},
    {('x'), QLatin1String("xrange")},
    {('z'), QLatin1String("zip")}
};

static const QMultiHash<char, QLatin1String> py_other = {
    {('i'), QLatin1String("import")}
};

void loadPythonData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other){
    types = py_types;
    keywords = py_keywords;
    builtin = py_builtin;
    literals = py_literals;
    other = py_other;
}

/********************************************************/
/***   Rust DATA      ***********************************/
/********************************************************/

static const QMultiHash<char, QLatin1String> rust_keywords = {
    {('a'), QLatin1String("abstract")},
    {('a'), QLatin1String("alignof")},
    {('a'), QLatin1String("as")},
    {('a'), QLatin1String("async")},
    {('a'), QLatin1String("await")},
    {('b'), QLatin1String("be")},
    {('b'), QLatin1String("box")},
    {('b'), QLatin1String("break")},
    {('c'), QLatin1String("const")},
    {('c'), QLatin1String("continue")},
    {('c'), QLatin1String("crate")},
    {('d'), QLatin1String("do")},
    {('d'), QLatin1String("dyn")},
    {('e'), QLatin1String("else")},
    {('e'), QLatin1String("extern")},
    {('f'), QLatin1String("final")},
    {('f'), QLatin1String("fn")},
    {('f'), QLatin1String("for")},
    {('i'), QLatin1String("if")},
    {('i'), QLatin1String("impl")},
    {('i'), QLatin1String("in")},
    {('l'), QLatin1String("let")},
    {('l'), QLatin1String("loop")},
    {('m'), QLatin1String("match")},
    {('m'), QLatin1String("mod")},
    {('m'), QLatin1String("move")},
    {('m'), QLatin1String("mut")},
    {('o'), QLatin1String("offsetof")},
    {('o'), QLatin1String("once")},
    {('o'), QLatin1String("override")},
    {('p'), QLatin1String("priv")},
    {('p'), QLatin1String("pub")},
    {('p'), QLatin1String("pure")},
    {('r'), QLatin1String("ref")},
    {('r'), QLatin1String("return")},
    {('s'), QLatin1String("sizeof")},
    {('s'), QLatin1String("static")},
    {('s'), QLatin1String("self")},
    {('S'), QLatin1String("Self")},
    {('s'), QLatin1String("super")},
    {('t'), QLatin1String("trait")},
    {('t'), QLatin1String("type")},
    {('t'), QLatin1String("typeof")},
    {('u'), QLatin1String("unsafe")},
    {('u'), QLatin1String("unsized")},
    {('u'), QLatin1String("use")},
    {('v'), QLatin1String("virtual")},
    {('w'), QLatin1String("where")},
    {('w'), QLatin1String("while")},
    {('y'), QLatin1String("yield")},
};

static const QMultiHash<char, QLatin1String> rust_types = {
    {('u'), QLatin1String("union")},
    {('e'), QLatin1String("enum")},
    {('s'), QLatin1String("struct")},

    {('i'), QLatin1String("i8")},
    {('i'), QLatin1String("i16")},
    {('i'), QLatin1String("i32")},
    {('i'), QLatin1String("i64")},
    {('i'), QLatin1String("i128")},
    {('i'), QLatin1String("isize")},
    {('u'), QLatin1String("u8")},
    {('u'), QLatin1String("u16")},
    {('u'), QLatin1String("u32")},
    {('u'), QLatin1String("u64")},
    {('u'), QLatin1String("u128")},
    {('u'), QLatin1String("usize")},
    {('f'), QLatin1String("f32")},
    {('f'), QLatin1String("f64")},
    {('s'), QLatin1String("str")},
    {('c'), QLatin1String("char")},
    {('b'), QLatin1String("bool")},
    {('B'), QLatin1String("Box")},
    {('O'), QLatin1String("Option")},
    {('R'), QLatin1String("Result")},
    {('S'), QLatin1String("String")},
    {('V'), QLatin1String("Vec")}
};

static const QMultiHash<char, QLatin1String> rust_literals = {
    {('f'), QLatin1String("false")},
    {('t'), QLatin1String("true")}
};

static const QMultiHash<char, QLatin1String> rust_builtin = {

};

static const QMultiHash<char, QLatin1String> rust_other = {
    {('a'), QLatin1String("assert!")},
    {('a'), QLatin1String("assert_eq!")},
    {('b'), QLatin1String("bitflags!")},
    {('b'), QLatin1String("bytes!")},
    {('c'), QLatin1String("cfg!")},
    {('c'), QLatin1String("col!")},
    {('c'), QLatin1String("concat!")},
    {('c'), QLatin1String("concat_idents!")},
    {('d'), QLatin1String("debug_assert!")},
    {('d'), QLatin1String("debug_assert_eq!")},
    {('e'), QLatin1String("env!")},
    {('p'), QLatin1String("panic!")},
    {('f'), QLatin1String("file!")},
    {('f'), QLatin1String("format!")},
    {('f'), QLatin1String("format_args!")},
    {('i'), QLatin1String("include_bin!")},
    {('i'), QLatin1String("include_str!")},
    {('l'), QLatin1String("line!")},
    {('l'), QLatin1String("local_data_key!")},
    {('m'), QLatin1String("module_path!")},
    {('o'), QLatin1String("option_env!")},
    {('p'), QLatin1String("print!")},
    {('p'), QLatin1String("println!")},
    {('s'), QLatin1String("select!")},
    {('s'), QLatin1String("stringify!")},
    {('t'), QLatin1String("try!")},
    {('u'), QLatin1String("unimplemented!")},
    {('u'), QLatin1String("unreachable!")},
    {('v'), QLatin1String("vec!")},
    {('w'), QLatin1String("write!")},
    {('w'), QLatin1String("writeln!")},
    {('m'), QLatin1String("macro_rules!")},
    {('a'), QLatin1String("assert_ne!")},
    {('d'), QLatin1String("debug_assert_ne!")}
};

void loadRustData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other){
    types = rust_types;
    keywords = rust_keywords;
    builtin = rust_builtin;
    literals = rust_literals;
    other = rust_other;
}

/********************************************************/
/***   Java DATA      ***********************************/
/********************************************************/

static const QMultiHash<char, QLatin1String> java_keywords = {
    {('a'), QLatin1String("abstract")},
    {('a'), QLatin1String("assert")},
    {('b'), QLatin1String("break")},
    {('c'), QLatin1String("case")},
    {('c'), QLatin1String("catch")},
    {('c'), QLatin1String("const")},
    {('c'), QLatin1String("continue")},
    {('d'), QLatin1String("default")},
    {('d'), QLatin1String("do")},
    {('e'), QLatin1String("else")},
    {('e'), QLatin1String("exports")},
    {('e'), QLatin1String("extends")},
    {('f'), QLatin1String("final")},
    {('f'), QLatin1String("finally")},
    {('f'), QLatin1String("for")},
    {('g'), QLatin1String("goto")},
    {('i'), QLatin1String("if")},
    {('i'), QLatin1String("implements")},
    {('i'), QLatin1String("import")},
    {('i'), QLatin1String("instanceof")},
    {('i'), QLatin1String("interface")},
    {('l'), QLatin1String("long")},
    {('m'), QLatin1String("module")},
    {('n'), QLatin1String("native")},
    {('n'), QLatin1String("new")},
    {('n'), QLatin1String("null")},
    {('o'), QLatin1String("open")},
    {('o'), QLatin1String("opens")},
    {('p'), QLatin1String("package")},
    {('p'), QLatin1String("private")},
    {('p'), QLatin1String("protected")},
    {('p'), QLatin1String("provides")},
    {('p'), QLatin1String("public")},
    {('r'), QLatin1String("requires")},
    {('r'), QLatin1String("return")},
    {('s'), QLatin1String("static")},
    {('s'), QLatin1String("strictfp")},
    {('s'), QLatin1String("super")},
    {('s'), QLatin1String("switch")},
    {('s'), QLatin1String("synchronized")},
    {('t'), QLatin1String("this")},
    {('t'), QLatin1String("throw")},
    {('t'), QLatin1String("throws")},
    {('t'), QLatin1String("to")},
    {('t'), QLatin1String("transient")},
    {('t'), QLatin1String("transitive")},
    {('t'), QLatin1String("try")},
    {('u'), QLatin1String("uses")},
    {('v'), QLatin1String("var")},
    {('v'), QLatin1String("volatile")},
    {('w'), QLatin1String("while")},
    {('w'), QLatin1String("with")},
    {('y'), QLatin1String("yield")}
};

static const QMultiHash<char, QLatin1String> java_types = {
    {('v'), QLatin1String("void")},
    {('f'), QLatin1String("float")},
    {('b'), QLatin1String("boolean")},
    {('b'), QLatin1String("byte")},
    {('i'), QLatin1String("int")},
    {('c'), QLatin1String("char")},
    {('c'), QLatin1String("class")},
    {('d'), QLatin1String("double")},
    {('e'), QLatin1String("enum")},
    {('s'), QLatin1String("short")},

};

static const QMultiHash<char, QLatin1String> java_literals = {
    {('f'), QLatin1String("false")},
    {('t'), QLatin1String("true")}
};

static const QMultiHash<char, QLatin1String> java_builtin = {

};

static const QMultiHash<char, QLatin1String> java_other = {

};

void loadJavaData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other){
    types = java_types;
    keywords = java_keywords;
    builtin = java_builtin;
    literals = java_literals;
    other = java_other;
}

/********************************************************/
/***   C# DATA      *************************************/
/********************************************************/

static const QMultiHash<char, QLatin1String> csharp_keywords = {
    {('a'), QLatin1String("abstract")},
    {('a'), QLatin1String("add")},
    {('a'), QLatin1String("alias")},
    {('a'), QLatin1String("as")},
    {('a'), QLatin1String("ascending")},
    {('a'), QLatin1String("async")},
    {('a'), QLatin1String("await")},
    {('b'), QLatin1String("base")},
    {('b'), QLatin1String("break")},
    {('c'), QLatin1String("case")},
    {('c'), QLatin1String("catch")},
    {('c'), QLatin1String("checked")},
    {('c'), QLatin1String("const")},
    {('c'), QLatin1String("continue")},
    {('d'), QLatin1String("decimal")},
    {('d'), QLatin1String("default")},
    {('d'), QLatin1String("delegate")},
    {('d'), QLatin1String("descending")},
    {('d'), QLatin1String("do")},
    {('d'), QLatin1String("dynamic")},
    {('e'), QLatin1String("else")},
    {('e'), QLatin1String("event")},
    {('e'), QLatin1String("explicit")},
    {('e'), QLatin1String("extern")},
    {('f'), QLatin1String("finally")},
    {('f'), QLatin1String("fixed")},
    {('f'), QLatin1String("for")},
    {('f'), QLatin1String("foreach")},
    {('f'), QLatin1String("from")},
    {('g'), QLatin1String("get")},
    {('g'), QLatin1String("global")},
    {('g'), QLatin1String("goto")},
    {('g'), QLatin1String("group")},
    {('i'), QLatin1String("if")},
    {('i'), QLatin1String("implicit")},
    {('i'), QLatin1String("in")},
    {('i'), QLatin1String("interface")},
    {('i'), QLatin1String("internal")},
    {('i'), QLatin1String("into")},
    {('i'), QLatin1String("is")},
    {('j'), QLatin1String("join")},
    {('l'), QLatin1String("let")},
    {('l'), QLatin1String("lock")},
    {('l'), QLatin1String("long")},
    {('n'), QLatin1String("namespace")},
    {('n'), QLatin1String("new")},
    {('o'), QLatin1String("object")},
    {('o'), QLatin1String("operator")},
    {('o'), QLatin1String("orderby")},
    {('o'), QLatin1String("out")},
    {('o'), QLatin1String("override")},
    {('p'), QLatin1String("params")},
    {('p'), QLatin1String("partial")},
    {('p'), QLatin1String("private")},
    {('p'), QLatin1String("protected")},
    {('p'), QLatin1String("public")},
    {('r'), QLatin1String("readonly")},
    {('r'), QLatin1String("ref")},
    {('r'), QLatin1String("remove")},
    {('r'), QLatin1String("return")},
    {('s'), QLatin1String("sealed")},
    {('s'), QLatin1String("select")},
    {('s'), QLatin1String("set")},
    {('s'), QLatin1String("sizeof")},
    {('s'), QLatin1String("stackalloc")},
    {('s'), QLatin1String("static")},
    {('s'), QLatin1String("switch")},
    {('t'), QLatin1String("this")},
    {('t'), QLatin1String("throw")},
    {('t'), QLatin1String("try")},
    {('t'), QLatin1String("typeof")},
    {('u'), QLatin1String("unchecked")},
    {('u'), QLatin1String("unsafe")},
    {('u'), QLatin1String("using")},
    {('v'), QLatin1String("value")},
    {('v'), QLatin1String("virtual")},
    {('v'), QLatin1String("volatile")},
    {('w'), QLatin1String("where")},
    {('w'), QLatin1String("while")},
    {('y'), QLatin1String("yield")}
};

static const QMultiHash<char, QLatin1String> csharp_types = {
    {('b'), QLatin1String("bool")},
    {('b'), QLatin1String("byte")},
    {('c'), QLatin1String("char")},
    {('c'), QLatin1String("class")},
    {('d'), QLatin1String("double")},
    {('e'), QLatin1String("enum")},
    {('f'), QLatin1String("float")},
    {('i'), QLatin1String("int")},
    {('s'), QLatin1String("sbyte")},
    {('s'), QLatin1String("short")},
    {('s'), QLatin1String("string")},
    {('s'), QLatin1String("struct")},
    {('u'), QLatin1String("uint")},
    {('u'), QLatin1String("ulong")},
    {('u'), QLatin1String("ushort")},
    {('v'), QLatin1String("var")},
    {('v'), QLatin1String("void")},
};

static const QMultiHash<char, QLatin1String> csharp_literals = {
    {('f'), QLatin1String("false")},
    {('t'), QLatin1String("true")},
    {('n'), QLatin1String("null")}
};

static const QMultiHash<char, QLatin1String> csharp_builtin = {

};

static const QMultiHash<char, QLatin1String> csharp_other = {
    {('d'), QLatin1String("define")},
    {('e'), QLatin1String("elif")},
    {('e'), QLatin1String("else")},
    {('e'), QLatin1String("endif")},
    {('e'), QLatin1String("endregion")},
    {('e'), QLatin1String("error")},
    {('i'), QLatin1String("if")},
    {('l'), QLatin1String("line")},
    {('p'), QLatin1String("pragma")},
    {('r'), QLatin1String("region")},
    {('u'), QLatin1String("undef")},
    {('w'), QLatin1String("warning")}
};

void loadCSharpData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other){
    types = csharp_types;
    keywords = csharp_keywords;
    builtin = csharp_builtin;
    literals = csharp_literals;
    other = csharp_other;
}

/********************************************************/
/***   Go DATA      *************************************/
/********************************************************/

static const QMultiHash<char, QLatin1String> go_keywords = {
    {('b'), QLatin1String("break")},
    {('c'), QLatin1String("case")},
    {('c'), QLatin1String("chan")},
    {('c'), QLatin1String("const")},
    {('c'), QLatin1String("continue")},
    {('d'), QLatin1String("default")},
    {('d'), QLatin1String("defer")},
    {('e'), QLatin1String("else")},
    {('f'), QLatin1String("fallthrough")},
    {('f'), QLatin1String("for")},
    {('f'), QLatin1String("func")},
    {('g'), QLatin1String("go")},
    {('t'), QLatin1String("to")},
    {('i'), QLatin1String("if")},
    {('i'), QLatin1String("import")},
    {('i'), QLatin1String("interface")},
    {('p'), QLatin1String("package")},
    {('r'), QLatin1String("range")},
    {('r'), QLatin1String("return")},
    {('s'), QLatin1String("select")},
    {('s'), QLatin1String("struct")},
    {('s'), QLatin1String("switch")},
    {('t'), QLatin1String("type")},
};

static const QMultiHash<char, QLatin1String> go_types = {
    {('m'), QLatin1String("map")},
    {('s'), QLatin1String("struct")},
    {('v'), QLatin1String("var")},
    {('b'), QLatin1String("bool")},
    {('b'), QLatin1String("byte")},
    {('c'), QLatin1String("complex64")},
    {('c'), QLatin1String("complex128")},
    {('f'), QLatin1String("float32")},
    {('f'), QLatin1String("float64")},
    {('i'), QLatin1String("int8")},
    {('i'), QLatin1String("int16")},
    {('i'), QLatin1String("int32")},
    {('i'), QLatin1String("int64")},
    {('s'), QLatin1String("string")},
    {('u'), QLatin1String("uint8")},
    {('u'), QLatin1String("uint16")},
    {('u'), QLatin1String("uint32")},
    {('u'), QLatin1String("uint64")},
    {('i'), QLatin1String("int")},
    {('u'), QLatin1String("uint")},
    {('u'), QLatin1String("uintptr")},
    {('r'), QLatin1String("rune")}
};

static const QMultiHash<char, QLatin1String> go_literals = {
    {('f'), QLatin1String("false")},
    {('t'), QLatin1String("true")},
    {('n'), QLatin1String("nil")},
    {('i'), QLatin1String("iota")}
};

static const QMultiHash<char, QLatin1String> go_builtin = {
    {('a'), QLatin1String("append")},
    {('c'), QLatin1String("cap")},
    {('c'), QLatin1String("close")},
    {('c'), QLatin1String("complex")},
    {('c'), QLatin1String("copy")},
    {('i'), QLatin1String("imag")},
    {('l'), QLatin1String("len")},
    {('m'), QLatin1String("make")},
    {('n'), QLatin1String("new")},
    {('p'), QLatin1String("panic")},
    {('p'), QLatin1String("print")},
    {('p'), QLatin1String("println")},
    {('r'), QLatin1String("real")},
    {('r'), QLatin1String("recover")},
    {('d'), QLatin1String("delete")}
};

static const QMultiHash<char, QLatin1String> go_other = {

};


void loadGoData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other) {
    types = go_types;
    keywords = go_keywords;
    builtin = go_builtin;
    literals = go_literals;
    other = go_other;
}

/********************************************************/
/***   V DATA      **************************************/
/********************************************************/

static const QMultiHash<char, QLatin1String> v_keywords = {
    {('b'), QLatin1String("break")},
    {('c'), QLatin1String("const")},
    {('c'), QLatin1String("continue")},
    {('d'), QLatin1String("defer")},
    {('e'), QLatin1String("else")},
    {('f'), QLatin1String("for")},
    {('f'), QLatin1String("fn")},
    {('g'), QLatin1String("go")},
    {('g'), QLatin1String("goto")},
    {('i'), QLatin1String("if")},
    {('i'), QLatin1String("import")},
    {('i'), QLatin1String("interface")},
    {('r'), QLatin1String("return")},
    {('s'), QLatin1String("struct")},
    {('s'), QLatin1String("switch")},
    {('t'), QLatin1String("type")},
    {('p'), QLatin1String("pub")},
    {('o'), QLatin1String("or")},
    {('n'), QLatin1String("none")}
};

static const QMultiHash<char, QLatin1String> v_types = {
    {('m'), QLatin1String("map")},
    {('s'), QLatin1String("struct")},
    {('b'), QLatin1String("bool")},
    {('b'), QLatin1String("byte")},
    {('f'), QLatin1String("f32")},
    {('f'), QLatin1String("f64")},
    {('i'), QLatin1String("i8")},
    {('i'), QLatin1String("i16")},
    {('i'), QLatin1String("int")},
    {('i'), QLatin1String("i64")},
    {('i'), QLatin1String("i128")},
    {('s'), QLatin1String("string")},
    {('u'), QLatin1String("u16")},
    {('u'), QLatin1String("u32")},
    {('u'), QLatin1String("u64")},
    {('u'), QLatin1String("u128")},
    {('u'), QLatin1String("byteptr")},
    {('u'), QLatin1String("voidptr")},
    {('r'), QLatin1String("rune")}
};

static const QMultiHash<char, QLatin1String> v_literals = {
    {('f'), QLatin1String("false")},
    {('t'), QLatin1String("true")},
};

static const QMultiHash<char, QLatin1String> v_builtin = {
};

static const QMultiHash<char, QLatin1String> v_other = {

};

void loadVData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other) {
    types = v_types;
    keywords = v_keywords;
    builtin = v_builtin;
    literals = v_literals;
    other = v_other;
}

/********************************************************/
/***   SQL DATA      ************************************/
/********************************************************/


static const QMultiHash<char, QLatin1String> sql_keywords = {
    {('A'), QLatin1String("ACTION")},
    {('A'), QLatin1String("ADD")},
    {('A'), QLatin1String("AFTER")},
    {('A'), QLatin1String("ALGORITHM")},
    {('A'), QLatin1String("ALL")},
    {('A'), QLatin1String("ALTER")},
    {('A'), QLatin1String("ANALYZE")},
    {('A'), QLatin1String("ANY")},
    {('A'), QLatin1String("APPLY")},
    {('A'), QLatin1String("AS")},
    {('A'), QLatin1String("ASC")},
    {('A'), QLatin1String("AUTHORIZATION")},
    {('A'), QLatin1String("AUTO_INCREMENT")},
    {('B'), QLatin1String("BACKUP")},
    {('B'), QLatin1String("BDB")},
    {('B'), QLatin1String("BEGIN")},
    {('B'), QLatin1String("BERKELEYDB")},
    {('B'), QLatin1String("BIGINT")},
    {('B'), QLatin1String("BINARY")},
    {('B'), QLatin1String("BIT")},
    {('B'), QLatin1String("BLOB")},
    {('B'), QLatin1String("BOOL")},
    {('B'), QLatin1String("BOOLEAN")},
    {('B'), QLatin1String("BREAK")},
    {('B'), QLatin1String("BROWSE")},
    {('B'), QLatin1String("BTREE")},
    {('B'), QLatin1String("BULK")},
    {('B'), QLatin1String("BY")},
    {('C'), QLatin1String("CALL")},
    {('C'), QLatin1String("CASCADED")},
    {('C'), QLatin1String("CASE")},
    {('C'), QLatin1String("CHAIN")},
    {('C'), QLatin1String("CHARACTER")},
    {('S'), QLatin1String("SET")},
    {('C'), QLatin1String("CHECKPOINT")},
    {('C'), QLatin1String("CLOSE")},
    {('C'), QLatin1String("CLUSTERED")},
    {('C'), QLatin1String("COALESCE")},
    {('C'), QLatin1String("COLLATE")},
    {('C'), QLatin1String("COLUMNS")},
    {('C'), QLatin1String("COMMENT")},
    {('C'), QLatin1String("COMMITTED")},
    {('C'), QLatin1String("COMPUTE")},
    {('C'), QLatin1String("CONNECT")},
    {('C'), QLatin1String("CONSISTENT")},
    {('C'), QLatin1String("CONSTRAINT")},
    {('C'), QLatin1String("CONTAINSTABLE")},
    {('C'), QLatin1String("CONTINUE")},
    {('C'), QLatin1String("CONVERT")},
    {('C'), QLatin1String("CREATE")},
    {('C'), QLatin1String("CROSS")},
    {('C'), QLatin1String("CURRENT_DATE")},
    {('_'), QLatin1String("_TIME")},
    {('_'), QLatin1String("_TIMESTAMP")},
    {('_'), QLatin1String("_USER")},
    {('C'), QLatin1String("CURSOR")},
    {('C'), QLatin1String("CYCLE")},
    {('D'), QLatin1String("DATABASES")},
    {('D'), QLatin1String("DATETIME")},
    {('D'), QLatin1String("DAY")},
    {('D'), QLatin1String("DBCC")},
    {('D'), QLatin1String("DEALLOCATE")},
    {('D'), QLatin1String("DEC")},
    {('D'), QLatin1String("DECIMAL")},
    {('D'), QLatin1String("DECLARE")},
    {('D'), QLatin1String("DEFAULT")},
    {('D'), QLatin1String("DEFINER")},
    {('D'), QLatin1String("DELAYED")},
    {('D'), QLatin1String("DELETE")},
    {('D'), QLatin1String("DELIMITERS")},
    {('D'), QLatin1String("DENY")},
    {('D'), QLatin1String("DESC")},
    {('D'), QLatin1String("DESCRIBE")},
    {('D'), QLatin1String("DETERMINISTIC")},
    {('D'), QLatin1String("DISABLE")},
    {('D'), QLatin1String("DISCARD")},
    {('D'), QLatin1String("DISK")},
    {('D'), QLatin1String("DISTINCT")},
    {('D'), QLatin1String("DISTINCTROW")},
    {('D'), QLatin1String("DISTRIBUTED")},
    {('D'), QLatin1String("DO")},
    {('D'), QLatin1String("DOUBLE")},
    {('D'), QLatin1String("DROP")},
    {('D'), QLatin1String("DUMMY")},
    {('D'), QLatin1String("DUMPFILE")},
    {('D'), QLatin1String("DUPLICATE")},
    {('E'), QLatin1String("ELSEIF")},
    {('E'), QLatin1String("ENABLE")},
    {('E'), QLatin1String("ENCLOSED")},
    {('E'), QLatin1String("END")},
    {('E'), QLatin1String("ENGINE")},
    {('E'), QLatin1String("ENUM")},
    {('E'), QLatin1String("ERRLVL")},
    {('E'), QLatin1String("ERRORS")},
    {('E'), QLatin1String("ESCAPED")},
    {('E'), QLatin1String("EXCEPT")},
    {('E'), QLatin1String("EXECUTE")},
    {('E'), QLatin1String("EXISTS")},
    {('E'), QLatin1String("EXIT")},
    {('E'), QLatin1String("EXPLAIN")},
    {('E'), QLatin1String("EXTENDED")},
    {('F'), QLatin1String("FETCH")},
    {('F'), QLatin1String("FIELDS")},
    {('F'), QLatin1String("FILE")},
    {('F'), QLatin1String("FILLFACTOR")},
    {('F'), QLatin1String("FIRST")},
    {('F'), QLatin1String("FIXED")},
    {('F'), QLatin1String("FLOAT")},
    {('F'), QLatin1String("FOLLOWING")},
    {('F'), QLatin1String("FOR")},
    {('E'), QLatin1String("EACH")},
    {('R'), QLatin1String("ROW")},
    {('F'), QLatin1String("FORCE")},
    {('F'), QLatin1String("FOREIGN")},
    {('F'), QLatin1String("FREETEXTTABLE")},
    {('F'), QLatin1String("FROM")},
    {('F'), QLatin1String("FULL")},
    {('F'), QLatin1String("FUNCTION")},
    {('G'), QLatin1String("GEOMETRYCOLLECTION")},
    {('G'), QLatin1String("GLOBAL")},
    {('G'), QLatin1String("GOTO")},
    {('G'), QLatin1String("GRANT")},
    {('G'), QLatin1String("GROUP")},
    {('H'), QLatin1String("HANDLER")},
    {('H'), QLatin1String("HASH")},
    {('H'), QLatin1String("HAVING")},
    {('H'), QLatin1String("HOLDLOCK")},
    {('H'), QLatin1String("HOUR")},
    {('I'), QLatin1String("IDENTITY_INSERT")},
    {('C'), QLatin1String("COL")},
    {('I'), QLatin1String("IF")},
    {('I'), QLatin1String("IGNORE")},
    {('I'), QLatin1String("IMPORT")},
    {('I'), QLatin1String("INDEX")},
    {('I'), QLatin1String("INFILE")},
    {('I'), QLatin1String("INNER")},
    {('I'), QLatin1String("INNODB")},
    {('I'), QLatin1String("INOUT")},
    {('I'), QLatin1String("INSERT")},
    {('I'), QLatin1String("INT")},
    {('I'), QLatin1String("INTEGER")},
    {('I'), QLatin1String("INTERSECT")},
    {('I'), QLatin1String("INTERVAL")},
    {('I'), QLatin1String("INTO")},
    {('I'), QLatin1String("INVOKER")},
    {('I'), QLatin1String("ISOLATION")},
    {('I'), QLatin1String("ITERATE")},
    {('J'), QLatin1String("JOIN")},
    {('K'), QLatin1String("KEYS")},
    {('K'), QLatin1String("KILL")},
    {('L'), QLatin1String("LANGUAGE")},
    {('L'), QLatin1String("LAST")},
    {('L'), QLatin1String("LEAVE")},
    {('L'), QLatin1String("LEFT")},
    {('L'), QLatin1String("LEVEL")},
    {('L'), QLatin1String("LIMIT")},
    {('L'), QLatin1String("LINENO")},
    {('L'), QLatin1String("LINES")},
    {('L'), QLatin1String("LINESTRING")},
    {('L'), QLatin1String("LOAD")},
    {('L'), QLatin1String("LOCAL")},
    {('L'), QLatin1String("LOCK")},
    {('L'), QLatin1String("LONGBLOB")},
    {('T'), QLatin1String("TEXT")},
    {('L'), QLatin1String("LOOP")},
    {('M'), QLatin1String("MATCHED")},
    {('M'), QLatin1String("MEDIUMBLOB")},
    {('I'), QLatin1String("INT")},
    {('T'), QLatin1String("TEXT")},
    {('M'), QLatin1String("MERGE")},
    {('M'), QLatin1String("MIDDLEINT")},
    {('M'), QLatin1String("MINUTE")},
    {('M'), QLatin1String("MODE")},
    {('M'), QLatin1String("MODIFIES")},
    {('M'), QLatin1String("MODIFY")},
    {('M'), QLatin1String("MONTH")},
    {('M'), QLatin1String("MULTILINESTRING")},
    {('P'), QLatin1String("POINT")},
    {('P'), QLatin1String("POLYGON")},
    {('N'), QLatin1String("NATIONAL")},
    {('N'), QLatin1String("NATURAL")},
    {('N'), QLatin1String("NCHAR")},
    {('N'), QLatin1String("NEXT")},
    {('N'), QLatin1String("NO")},
    {('N'), QLatin1String("NONCLUSTERED")},
    {('N'), QLatin1String("NULLIF")},
    {('N'), QLatin1String("NUMERIC")},
    {('O'), QLatin1String("OFF")},
    {('O'), QLatin1String("OFFSETS")},
    {('O'), QLatin1String("ON")},
    {('O'), QLatin1String("OPENDATASOURCE")},
    {('Q'), QLatin1String("QUERY")},
    {('R'), QLatin1String("ROWSET")},
    {('O'), QLatin1String("OPTIMIZE")},
    {('O'), QLatin1String("OPTIONALLY")},
    {('O'), QLatin1String("ORDER")},
    {('O'), QLatin1String("OUTER")},
    {('F'), QLatin1String("FILE")},
    {('O'), QLatin1String("OVER")},
    {('P'), QLatin1String("PARTIAL")},
    {('P'), QLatin1String("PARTITION")},
    {('P'), QLatin1String("PERCENT")},
    {('P'), QLatin1String("PIVOT")},
    {('P'), QLatin1String("PLAN")},
    {('P'), QLatin1String("POINT")},
    {('P'), QLatin1String("POLYGON")},
    {('P'), QLatin1String("PRECEDING")},
    {('P'), QLatin1String("PRECISION")},
    {('P'), QLatin1String("PREPARE")},
    {('P'), QLatin1String("PREV")},
    {('P'), QLatin1String("PRIMARY")},
    {('P'), QLatin1String("PRINT")},
    {('P'), QLatin1String("PRIVILEGES")},
    {('P'), QLatin1String("PROCEDURE")},
    {('P'), QLatin1String("PUBLIC")},
    {('P'), QLatin1String("PURGE")},
    {('Q'), QLatin1String("QUICK")},
    {('R'), QLatin1String("RAISERROR")},
    {('R'), QLatin1String("READS")},
    {('R'), QLatin1String("REAL")},
    {('R'), QLatin1String("RECONFIGURE")},
    {('R'), QLatin1String("REFERENCES")},
    {('R'), QLatin1String("RELEASE")},
    {('R'), QLatin1String("RENAME")},
    {('R'), QLatin1String("REPEATABLE")},
    {('R'), QLatin1String("REPLACE")},
    {('R'), QLatin1String("REPLICATION")},
    {('R'), QLatin1String("REQUIRE")},
    {('R'), QLatin1String("RESIGNAL")},
    {('R'), QLatin1String("RESTORE")},
    {('R'), QLatin1String("RESTRICT")},
    {('R'), QLatin1String("RETURNS")},
    {('R'), QLatin1String("REVOKE")},
    {('R'), QLatin1String("RIGHT")},
    {('R'), QLatin1String("ROLLBACK")},
    {('R'), QLatin1String("ROUTINE")},
    {('R'), QLatin1String("ROWCOUNT")},
    {('G'), QLatin1String("GUIDCOL")},
    {('R'), QLatin1String("RTREE")},
    {('R'), QLatin1String("RULE")},
    {('S'), QLatin1String("SAVEPOINT")},
    {('S'), QLatin1String("SCHEMA")},
    {('S'), QLatin1String("SECOND")},
    {('S'), QLatin1String("SELECT")},
    {('S'), QLatin1String("SERIALIZABLE")},
    {('S'), QLatin1String("SESSION_USER")},
    {('S'), QLatin1String("SETUSER")},
    {('S'), QLatin1String("SHARE")},
    {('S'), QLatin1String("SHOW")},
    {('S'), QLatin1String("SHUTDOWN")},
    {('S'), QLatin1String("SIMPLE")},
    {('S'), QLatin1String("SMALLINT")},
    {('S'), QLatin1String("SNAPSHOT")},
    {('S'), QLatin1String("SOME")},
    {('S'), QLatin1String("SONAME")},
    {('S'), QLatin1String("SQL")},
    {('S'), QLatin1String("STARTING")},
    {('S'), QLatin1String("STATISTICS")},
    {('S'), QLatin1String("STATUS")},
    {('S'), QLatin1String("STRIPED")},
    {('S'), QLatin1String("SYSTEM_USER")},
    {('T'), QLatin1String("TABLES")},
    {('T'), QLatin1String("TABLESPACE")},
    {('T'), QLatin1String("TEMPORARY")},
    {('T'), QLatin1String("TABLE")},
    {('T'), QLatin1String("TERMINATED")},
    {('T'), QLatin1String("TEXTSIZE")},
    {('T'), QLatin1String("THEN")},
    {('T'), QLatin1String("TIMESTAMP")},
    {('T'), QLatin1String("TINYBLOB")},
    {('I'), QLatin1String("INT")},
    {('T'), QLatin1String("TEXT")},
    {('T'), QLatin1String("TOP")},
    {('T'), QLatin1String("TRANSACTIONS")},
    {('T'), QLatin1String("TRIGGER")},
    {('T'), QLatin1String("TRUNCATE")},
    {('T'), QLatin1String("TSEQUAL")},
    {('T'), QLatin1String("TYPES")},
    {('U'), QLatin1String("UNBOUNDED")},
    {('U'), QLatin1String("UNCOMMITTED")},
    {('U'), QLatin1String("UNDEFINED")},
    {('U'), QLatin1String("UNION")},
    {('U'), QLatin1String("UNIQUE")},
    {('U'), QLatin1String("UNLOCK")},
    {('U'), QLatin1String("UNPIVOT")},
    {('U'), QLatin1String("UNSIGNED")},
    {('U'), QLatin1String("UPDATETEXT")},
    {('U'), QLatin1String("USAGE")},
    {('U'), QLatin1String("USE")},
    {('U'), QLatin1String("USER")},
    {('U'), QLatin1String("USING")},
    {('V'), QLatin1String("VALUES")},
    {('V'), QLatin1String("VARBINARY")},
    {('C'), QLatin1String("CHAR")},
    {('C'), QLatin1String("CHARACTER")},
    {('Y'), QLatin1String("YING")},
    {('V'), QLatin1String("VIEW")},
    {('W'), QLatin1String("WAITFOR")},
    {('W'), QLatin1String("WARNINGS")},
    {('W'), QLatin1String("WHEN")},
    {('W'), QLatin1String("WHERE")},
    {('W'), QLatin1String("WHILE")},
    {('W'), QLatin1String("WITH")},
    {('R'), QLatin1String("ROLLUP")},
    {('I'), QLatin1String("IN")},
    {('W'), QLatin1String("WORK")},
    {('W'), QLatin1String("WRITETEXT")},
    {('Y'), QLatin1String("YEAR")}
};

static const QMultiHash<char, QLatin1String> sql_types = {

};

static const QMultiHash<char, QLatin1String> sql_literals = {
    {('A'), QLatin1String("TRUE")},
    {('F'), QLatin1String("FALSE")},
    {('N'), QLatin1String("NULL")},
};

static const QMultiHash<char, QLatin1String> sql_builtin = {
    {('A'), QLatin1String("AVG")},
    {('C'), QLatin1String("COUNT")},
    {('F'), QLatin1String("FIRST")},
    {('F'), QLatin1String("FORMAT")},
    {('L'), QLatin1String("LAST")},
    {('L'), QLatin1String("LCASE")},
    {('L'), QLatin1String("LEN")},
    {('M'), QLatin1String("MAX")},
    {('M'), QLatin1String("MID")},
    {('M'), QLatin1String("MIN")},
    {('M'), QLatin1String("MOD")},
    {('N'), QLatin1String("NOW")},
    {('R'), QLatin1String("ROUND")},
    {('S'), QLatin1String("SUM")},
    {('U'), QLatin1String("UCASE")}
};

static const QMultiHash<char, QLatin1String> sql_other = {

};

void loadSQLData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other){
    types = sql_types;
    keywords = sql_keywords;
    builtin = sql_builtin;
    literals = sql_literals;
    other = sql_other;
}

/********************************************************/
/***   JSON DATA      ***********************************/
/********************************************************/

static const QMultiHash<char, QLatin1String> json_keywords = {
};

static const QMultiHash<char, QLatin1String> json_types = {
};

static const QMultiHash<char, QLatin1String> json_literals = {
    {('f'), QLatin1String("false")},
    {('t'), QLatin1String("true")},
    {('n'), QLatin1String("null")}
};

static const QMultiHash<char, QLatin1String> json_builtin = {
};

static const QMultiHash<char, QLatin1String> json_other = {
};

void loadJSONData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other){
    types = json_types;
    keywords = json_keywords;
    builtin = json_builtin;
    literals = json_literals;
    other = json_other;
}

/********************************************************/
/***   CSS DATA      ***********************************/
/********************************************************/

static const QMultiHash<char, QLatin1String> css_keywords = {
    {'i', QLatin1String("important")},
    {'p', QLatin1String("px")},
    {'e', QLatin1String("em")}
};

static const QMultiHash<char, QLatin1String> css_types = {
    {'a', QLatin1String("align")},
    {'c', QLatin1String("content")},
    {'i', QLatin1String("items")},
    {'s', QLatin1String("self")},
    {'a', QLatin1String("all")},
    {'a', QLatin1String("animation")},
    {'d', QLatin1String("delay")},
    {'d', QLatin1String("direction")},
    {'d', QLatin1String("duration")},
    {'f', QLatin1String("fill")},
    {'m', QLatin1String("mode")},
    {'i', QLatin1String("iteration")},
    {'c', QLatin1String("count")},
    {'n', QLatin1String("name")},
    {'p', QLatin1String("play")},
    {'s', QLatin1String("state")},
    {'t', QLatin1String("timing")},
    {'f', QLatin1String("function")},
    {'a', QLatin1String("azimuth")},
    {'b', QLatin1String("backface")},
    {'v', QLatin1String("visibility")},
    {'a', QLatin1String("attachment")},
    {'b', QLatin1String("blend")},
    {'m', QLatin1String("mode")},
    {'c', QLatin1String("clip")},
    {'c', QLatin1String("color")},
    {'i', QLatin1String("image")},
    {'o', QLatin1String("origin")},
    {'p', QLatin1String("position")},
    {'r', QLatin1String("repeat")},
    {'s', QLatin1String("size")},
    {'b', QLatin1String("background")},
    {'b', QLatin1String("bleed")},
    {'c', QLatin1String("color")},
    {'r', QLatin1String("radius")},
    {'r', QLatin1String("radius")},
    {'s', QLatin1String("style")},
    {'w', QLatin1String("width")},
    {'b', QLatin1String("bottom")},
    {'c', QLatin1String("collapse")},
    {'c', QLatin1String("color")},
    {'i', QLatin1String("image")},
    {'o', QLatin1String("outset")},
    {'r', QLatin1String("repeat")},
    {'s', QLatin1String("source")},
    {'s', QLatin1String("slice")},
    {'w', QLatin1String("width")},
    {'c', QLatin1String("color")},
    {'s', QLatin1String("style")},
    {'w', QLatin1String("width")},
    {'l', QLatin1String("left")},
    {'r', QLatin1String("radius")},
    {'c', QLatin1String("color")},
    {'s', QLatin1String("style")},
    {'w', QLatin1String("width")},
    {'r', QLatin1String("right")},
    {'s', QLatin1String("spacing")},
    {'s', QLatin1String("style")},
    {'c', QLatin1String("color")},
    {'l', QLatin1String("left")},
    {'r', QLatin1String("radius")},
    {'r', QLatin1String("radius")},
    {'s', QLatin1String("style")},
    {'w', QLatin1String("width")},
    {'t', QLatin1String("top")},
    {'w', QLatin1String("width")},
    {'b', QLatin1String("border")},
    {'b', QLatin1String("bottom")},
    {'b', QLatin1String("break")},
    {'b', QLatin1String("box")},
    {'s', QLatin1String("shadow")},
    {'b', QLatin1String("box")},
    {'s', QLatin1String("sizing")},
    {'a', QLatin1String("after")},
    {'b', QLatin1String("before")},
    {'b', QLatin1String("break")},
    {'i', QLatin1String("inside")},
    {'c', QLatin1String("caption")},
    {'s', QLatin1String("side")},
    {'c', QLatin1String("caret")},
    {'c', QLatin1String("color")},
    {'c', QLatin1String("clear")},
    {'c', QLatin1String("clip")},
    {'c', QLatin1String("color")},
    {'c', QLatin1String("columns")},
    {'c', QLatin1String("column")},
    {'c', QLatin1String("count")},
    {'f', QLatin1String("fill")},
    {'g', QLatin1String("gap")},
    {'r', QLatin1String("rule")},
    {'c', QLatin1String("color")},
    {'s', QLatin1String("style")},
    {'w', QLatin1String("width")},
    {'s', QLatin1String("span")},
    {'w', QLatin1String("width")},
    {'c', QLatin1String("content")},
    {'i', QLatin1String("increment")},
    {'c', QLatin1String("counter")},
    {'r', QLatin1String("reset")},
    {'a', QLatin1String("after")},
    {'b', QLatin1String("before")},
    {'c', QLatin1String("cue")},
    {'c', QLatin1String("cursor")},
    {'d', QLatin1String("direction")},
    {'d', QLatin1String("display")},
    {'e', QLatin1String("elevation")},
    {'e', QLatin1String("empty")},
    {'c', QLatin1String("cells")},
    {'f', QLatin1String("filter")},
    {'f', QLatin1String("flex")},
    {'b', QLatin1String("basis")},
    {'d', QLatin1String("direction")},
    {'f', QLatin1String("feature")},
    {'s', QLatin1String("settings")},
    {'f', QLatin1String("flex")},
    {'f', QLatin1String("flow")},
    {'g', QLatin1String("grow")},
    {'s', QLatin1String("shrink")},
    {'w', QLatin1String("wrap")},
    {'f', QLatin1String("float")},
    {'f', QLatin1String("family")},
    {'k', QLatin1String("kerning")},
    {'l', QLatin1String("language")},
    {'o', QLatin1String("override")},
    {'a', QLatin1String("adjust")},
    {'s', QLatin1String("size")},
    {'s', QLatin1String("stretch")},
    {'s', QLatin1String("style")},
    {'s', QLatin1String("synthesis")},
    {'v', QLatin1String("variant")},
    {'a', QLatin1String("alternates")},
    {'c', QLatin1String("caps")},
    {'e', QLatin1String("east")},
    {'a', QLatin1String("asian")},
    {'l', QLatin1String("ligatures")},
    {'n', QLatin1String("numeric")},
    {'p', QLatin1String("position")},
    {'w', QLatin1String("weight")},
    {'f', QLatin1String("font")},
    {'a', QLatin1String("area")},
    {'a', QLatin1String("auto")},
    {'c', QLatin1String("columns")},
    {'f', QLatin1String("flow")},
    {'r', QLatin1String("rows")},
    {'e', QLatin1String("end")},
    {'g', QLatin1String("gap")},
    {'s', QLatin1String("start")},
    {'c', QLatin1String("column")},
    {'g', QLatin1String("gap")},
    {'e', QLatin1String("end")},
    {'g', QLatin1String("gap")},
    {'s', QLatin1String("start")},
    {'r', QLatin1String("row")},
    {'a', QLatin1String("areas")},
    {'c', QLatin1String("columns")},
    {'r', QLatin1String("rows")},
    {'t', QLatin1String("template")},
    {'g', QLatin1String("grid")},
    {'h', QLatin1String("hanging")},
    {'p', QLatin1String("punctuation")},
    {'h', QLatin1String("height")},
    {'h', QLatin1String("hyphens")},
    {'i', QLatin1String("isolation")},
    {'j', QLatin1String("justify")},
    {'c', QLatin1String("content")},
    {'i', QLatin1String("items")},
    {'s', QLatin1String("self")},
    {'l', QLatin1String("leftimage")},
    {'l', QLatin1String("letter")},
    {'s', QLatin1String("spacing")},
    {'b', QLatin1String("break")},
    {'l', QLatin1String("line")},
    {'s', QLatin1String("style")},
    {'i', QLatin1String("image")},
    {'s', QLatin1String("style")},
    {'p', QLatin1String("position")},
    {'t', QLatin1String("type")},
    {'l', QLatin1String("list")},
    {'s', QLatin1String("style")},
    {'b', QLatin1String("bottom")},
    {'l', QLatin1String("left")},
    {'r', QLatin1String("right")},
    {'t', QLatin1String("top")},
    {'m', QLatin1String("margin")},
    {'m', QLatin1String("marker")},
    {'o', QLatin1String("offset")},
    {'m', QLatin1String("marks")},
    {'m', QLatin1String("max")},
    {'h', QLatin1String("height")},
    {'w', QLatin1String("width")},
    {'m', QLatin1String("min")},
    {'m', QLatin1String("mix")},
    {'b', QLatin1String("blend")},
    {'m', QLatin1String("mode")},
    {'n', QLatin1String("nav")},
    {'u', QLatin1String("up")},
    {'d', QLatin1String("down")},
    {'l', QLatin1String("left")},
    {'r', QLatin1String("right")},
    {'o', QLatin1String("opacity")},
    {'o', QLatin1String("order")},
    {'o', QLatin1String("orphans")},
    {'c', QLatin1String("color")},
    {'o', QLatin1String("offset")},
    {'s', QLatin1String("style")},
    {'w', QLatin1String("width")},
    {'o', QLatin1String("outline")},
    {'w', QLatin1String("wrap")},
    {'o', QLatin1String("overflow")},
    {'b', QLatin1String("bottom")},
    {'l', QLatin1String("left")},
    {'r', QLatin1String("right")},
    {'t', QLatin1String("top")},
    {'p', QLatin1String("padding")},
    {'b', QLatin1String("break")},
    {'a', QLatin1String("after")},
    {'b', QLatin1String("before")},
    {'i', QLatin1String("inside")},
    {'p', QLatin1String("page")},
    {'a', QLatin1String("after")},
    {'b', QLatin1String("before")},
    {'p', QLatin1String("pause")},
    {'p', QLatin1String("perspective")},
    {'o', QLatin1String("origin")},
    {'r', QLatin1String("range")},
    {'p', QLatin1String("pitch")},
    {'c', QLatin1String("content")},
    {'i', QLatin1String("items")},
    {'p', QLatin1String("place")},
    {'s', QLatin1String("self")},
    {'p', QLatin1String("play")},
    {'d', QLatin1String("during")},
    {'p', QLatin1String("position")},
    {'q', QLatin1String("quotes")},
    {'r', QLatin1String("resize")},
    {'r', QLatin1String("rest")},
    {'a', QLatin1String("after")},
    {'b', QLatin1String("before")},
    {'r', QLatin1String("rest")},
    {'r', QLatin1String("richness")},
    {'r', QLatin1String("right")},
    {'s', QLatin1String("size")},
    {'h', QLatin1String("header")},
    {'n', QLatin1String("numeral")},
    {'s', QLatin1String("speak")},
    {'p', QLatin1String("punctuation")},
    {'s', QLatin1String("speak")},
    {'s', QLatin1String("speech")},
    {'r', QLatin1String("rate")},
    {'s', QLatin1String("stress")},
    {'t', QLatin1String("tab")},
    {'s', QLatin1String("size")},
    {'t', QLatin1String("table")},
    {'l', QLatin1String("layout")},
    {'t', QLatin1String("text")},
    {'a', QLatin1String("align")},
    {'l', QLatin1String("last")},
    {'d', QLatin1String("decoration")},
    {'c', QLatin1String("color")},
    {'l', QLatin1String("line")},
    {'s', QLatin1String("skip")},
    {'s', QLatin1String("style")},
    {'i', QLatin1String("indent")},
    {'o', QLatin1String("overflow")},
    {'s', QLatin1String("shadow")},
    {'t', QLatin1String("transform")},
    {'u', QLatin1String("underline")},
    {'p', QLatin1String("position")},
    {'t', QLatin1String("top")},
    {'t', QLatin1String("transform")},
    {'o', QLatin1String("origin")},
    {'s', QLatin1String("style")},
    {'t', QLatin1String("transition")},
    {'d', QLatin1String("delay")},
    {'d', QLatin1String("duration")},
    {'p', QLatin1String("property")},
    {'t', QLatin1String("timing")},
    {'f', QLatin1String("function")},
    {'u', QLatin1String("unicode")},
    {'b', QLatin1String("bidi")},
    {'v', QLatin1String("vertical")},
    {'a', QLatin1String("align")},
    {'v', QLatin1String("visibility")},
    {'b', QLatin1String("balance")},
    {'d', QLatin1String("duration")},
    {'f', QLatin1String("family")},
    {'p', QLatin1String("pitch")},
    {'r', QLatin1String("range")},
    {'r', QLatin1String("rate")},
    {'s', QLatin1String("stress")},
    {'v', QLatin1String("volume")},
    {'v', QLatin1String("voice")},
    {'v', QLatin1String("volume")},
    {'w', QLatin1String("white")},
    {'s', QLatin1String("space")},
    {'w', QLatin1String("widows")},
    {'w', QLatin1String("width")},
    {'w', QLatin1String("will")},
    {'c', QLatin1String("change")},
    {'w', QLatin1String("word")},
    {'b', QLatin1String("break")},
    {'s', QLatin1String("spacing")},
    {'w', QLatin1String("wrap")},
    {'x', QLatin1String("x")},
    {'y', QLatin1String("y")},
    {'z', QLatin1String("z")},
    {'i', QLatin1String("index")},
    {'r', QLatin1String("rgb")},
    {'s', QLatin1String("sans")},
    {'s', QLatin1String("serif")},
    {'n', QLatin1String("normal")}
};

static const QMultiHash<char, QLatin1String> css_literals = {
};

static const QMultiHash<char, QLatin1String> css_builtin = {
};

static const QMultiHash<char, QLatin1String> css_other = {
};

void loadCSSData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other){
    types = css_types;
    keywords = css_keywords;
    builtin = css_builtin;
    literals = css_literals;
    other = css_other;
}

/********************************************************/
/***   Typescript DATA  *********************************/
/********************************************************/

static const QMultiHash<char, QLatin1String> typescript_keywords = {
    {'i', QLatin1String("in")},
    {'i', QLatin1String("if")},
    {'f', QLatin1String("for")},
    {'w', QLatin1String("while")},
    {'f', QLatin1String("finally")},
    {'n', QLatin1String("new")},
    {'f', QLatin1String("function")},
    {'d', QLatin1String("do")},
    {'r', QLatin1String("return")},
    {'v', QLatin1String("void")},
    {'e', QLatin1String("else")},
    {'b', QLatin1String("break")},
    {'c', QLatin1String("catch")},
    {'i', QLatin1String("instanceof")},
    {'w', QLatin1String("with")},
    {'t', QLatin1String("throw")},
    {'c', QLatin1String("case")},
    {'d', QLatin1String("default")},
    {'t', QLatin1String("try")},
    {'t', QLatin1String("this")},
    {'s', QLatin1String("switch")},
    {'c', QLatin1String("continue")},
    {'t', QLatin1String("typeof")},
    {'d', QLatin1String("delete")},
    {'l', QLatin1String("let")},
    {'y', QLatin1String("yield")},
    {'c', QLatin1String("const")},
    {'p', QLatin1String("public")},
    {'p', QLatin1String("private")},
    {'p', QLatin1String("protected")},
    {'g', QLatin1String("get")},
    {'s', QLatin1String("set")},
    {'s', QLatin1String("super")},
    {'s', QLatin1String("static")},
    {'i', QLatin1String("implements")},
    {'e', QLatin1String("export")},
    {'i', QLatin1String("import")},
    {'d', QLatin1String("declare")},
    {'t', QLatin1String("type")},
    {'n', QLatin1String("namespace")},
    {'a', QLatin1String("abstract")},
    {'a', QLatin1String("as")},
    {'f', QLatin1String("from")},
    {'e', QLatin1String("extends")},
    {'a', QLatin1String("async")},
    {'a', QLatin1String("await")}
};

static const QMultiHash<char, QLatin1String> typescript_types = {
    {'v', QLatin1String("var")},
    {'c', QLatin1String("class")},
    {'e', QLatin1String("enum")}
};

static const QMultiHash<char, QLatin1String> typescript_literals = {
    {('f'), QLatin1String("false")},
    {('n'), QLatin1String("null")},
    {('t'), QLatin1String("true")},
    {('u'), QLatin1String("undefined")},
    {('N'), QLatin1String("NaN")},
    {('I'), QLatin1String("Infinity")}
};

static const QMultiHash<char, QLatin1String> typescript_builtin = {
    {'e', QLatin1String("eval")},
    {'i', QLatin1String("isFinite")},
    {'i', QLatin1String("isNaN")},
    {'p', QLatin1String("parseFloat")},
    {'p', QLatin1String("parseInt")},
    {'d', QLatin1String("decodeURI")},
    {'d', QLatin1String("decodeURIComponent")},
    {'e', QLatin1String("encodeURI")},
    {'e', QLatin1String("encodeURIComponent")},
    {'e', QLatin1String("escape")},
    {'u', QLatin1String("unescape")},
    {'O', QLatin1String("Object")},
    {'F', QLatin1String("Function")},
    {'B', QLatin1String("Boolean")},
    {'E', QLatin1String("Error")},
    {'E', QLatin1String("EvalError")},
    {'I', QLatin1String("InternalError")},
    {'R', QLatin1String("RangeError")},
    {'R', QLatin1String("ReferenceError")},
    {'S', QLatin1String("StopIteration")},
    {'S', QLatin1String("SyntaxError")},
    {'T', QLatin1String("TypeError")},
    {'U', QLatin1String("URIError")},
    {'N', QLatin1String("Number")},
    {'M', QLatin1String("Math")},
    {'D', QLatin1String("Date")},
    {'S', QLatin1String("String")},
    {'R', QLatin1String("RegExp")},
    {'A', QLatin1String("Array")},
    {'F', QLatin1String("Float32Array")},
    {'F', QLatin1String("Float64Array")},
    {'I', QLatin1String("Int16Array")},
    {'I', QLatin1String("Int32Array")},
    {'I', QLatin1String("Int8Array")},
    {'U', QLatin1String("Uint16Array")},
    {'U', QLatin1String("Uint32Array")},
    {'U', QLatin1String("Uint8Array")},
    {'U', QLatin1String("Uint8ClampedArray")},
    {'A', QLatin1String("ArrayBuffer")},
    {'D', QLatin1String("DataView")},
    {'J', QLatin1String("JSON")},
    {'I', QLatin1String("Intl")},
    {'a', QLatin1String("arguments")},
    {'r', QLatin1String("require")},
    {'m', QLatin1String("module")},
    {'c', QLatin1String("console")},
    {'w', QLatin1String("window")},
    {'d', QLatin1String("document")},
    {'a', QLatin1String("any")},
    {'n', QLatin1String("number")},
    {'b', QLatin1String("boolean")},
    {'s', QLatin1String("string")},
    {'v', QLatin1String("void")},
    {'P', QLatin1String("Promise")}
};

static const QMultiHash<char, QLatin1String> typescript_other = {
};

void loadTypescriptData(QMultiHash<char, QLatin1String> &types,
             QMultiHash<char, QLatin1String> &keywords,
             QMultiHash<char, QLatin1String> &builtin,
             QMultiHash<char, QLatin1String> &literals,
             QMultiHash<char, QLatin1String> &other){
    types = typescript_types;
    keywords = typescript_keywords;
    builtin = typescript_builtin;
    literals = typescript_literals;
    other = typescript_other;
}
