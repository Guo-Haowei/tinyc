#ifndef NODE
#define NODE(name, dname, stmt, expr)
#endif // #ifndef NODE

//   enum,          name,               stmt?   expr?
NODE(INVALID,       "<error>",          0,      0)
NODE(PROG,          "<prog>",           0,      0)

NODE(STMT_CMP,      "<comp-stmt>",      1,      0)
NODE(STMT_RET,      "<ret-stmt>",       1,      0)

NODE(EXPR_LIT,      "<expr-literal>",   0,      1)
