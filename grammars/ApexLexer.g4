lexer grammar ApexLexer;

// Keywords (Case-Insensitive handling)
options {
    caseInsensitive = true;
}

channels {
    WHITESPACE_CHANNEL,
    COMMENT_CHANNEL
}

// Access Modifiers & Class Keywords
ABSTRACT        : 'abstract';
GLOBAL          : 'global';
PUBLIC          : 'public';
PROTECTED       : 'protected';
PRIVATE         : 'private';
TRANSIENT       : 'transient';
STATIC          : 'static';
FINAL           : 'final';
VIRTUAL         : 'virtual';
OVERRIDE        : 'override';
TESTMETHOD      : 'testmethod';
WEBSERVICE      : 'webservice';
WITH_SHARING    : 'with' [ \t\r\n]+ 'sharing';
WITHOUT_SHARING : 'without' [ \t\r\n]+ 'sharing';
INHERITED_SHARING: 'inherited' [ \t\r\n]+ 'sharing';

CLASS           : 'class';
INTERFACE       : 'interface';
EXTENDS         : 'extends';
IMPLEMENTS      : 'implements';
ENUM            : 'enum';

// Control Flow
IF              : 'if';
ELSE            : 'else';
SWITCH          : 'switch';
ON              : 'on';
WHEN            : 'when';
WHILE           : 'while';
DO              : 'do';
FOR             : 'for';
BREAK           : 'break';
CONTINUE        : 'continue';
RETURN          : 'return';
TRY             : 'try';
CATCH           : 'catch';
FINALLY         : 'finally';
THROW           : 'throw';

// OOP & Language Constructs
THIS            : 'this';
SUPER           : 'super';
NEW             : 'new';
INSTANCEOF      : 'instanceof';
GET             : 'get';
SET             : 'set';
TRIGGER         : 'trigger';
BEFORE          : 'before';
AFTER           : 'after';
SYSTEM          : 'system';
SYSTEMRUNAS     : 'system' [ \t\r\n]* '.' [ \t\r\n]* 'runas';

// DML Keywords
INSERT          : 'insert';
UPDATE          : 'update';
UPSERT          : 'upsert';
DELETE          : 'delete';
UNDELETE        : 'undelete';
MERGE           : 'merge';

// SOQL & SOSL Keywords
SELECT          : 'select';
COUNT           : 'count';
FROM            : 'from';
AS              : 'as';
USING           : 'using';
SCOPE           : 'scope';
WHERE           : 'where';
ORDER           : 'order';
BY              : 'by';
LIMIT           : 'limit';
OFFSET          : 'offset';
ASC             : 'asc';
DESC            : 'desc';
NULLS           : 'nulls';
FIRST           : 'first';
LAST            : 'last';
AND             : 'and';
OR              : 'or';
NOT             : 'not';
IN              : 'in';
LIKE            : 'like';
INCLUDES        : 'includes';
EXCLUDES        : 'excludes';
GROUP           : 'group';
ALL             : 'all';
ROWS            : 'rows';
VIEW            : 'view';
HAVING          : 'having';
ROLLUP          : 'rollup';
CUBE            : 'cube';
AVG             : 'avg';
COUNT_DISTINCT  : 'count_distinct';
MIN             : 'min';
MAX             : 'max';
SUM             : 'sum';
TYPEOF          : 'typeof';
THEN            : 'then';
END             : 'end';
TOLABEL         : 'tolabel';
FORMAT          : 'format';
TRACKING        : 'tracking';
VIEWSTAT        : 'viewstat';
DATA            : 'data';
CATEGORY        : 'category';
AT              : 'at';
ABOVE           : 'above';
BELOW           : 'below';
ABOVE_OR_BELOW  : 'above_or_below';
SECURITY_ENFORCED : 'security_enforced';
USER_MODE       : 'user_mode';
SYSTEM_MODE     : 'system_mode';
USER            : 'user';
FIELDS          : 'fields';
STANDARD        : 'standard';
CUSTOM          : 'custom';

// Literals
NULL_LITERAL    : 'null';
BOOLEAN_LITERAL : 'true' | 'false';

// Primitive & Built-in Types
VOID            : 'void';
BLOB            : 'blob';
BOOLEAN         : 'boolean';
DATE            : 'date';
DATETIME        : 'datetime';
DECIMAL         : 'decimal';
DOUBLE          : 'double';
ID_TYPE         : 'id';
INTEGER         : 'integer';
LONG            : 'long';
OBJECT          : 'object';
STRING          : 'string';
TIME            : 'time';
LIST            : 'list';
SET_TYPE        : 'set';
MAP             : 'map';

// Operators & Punctuation
LPAREN          : '(';
RPAREN          : ')';
LBRACE          : '{';
RBRACE          : '}';
LBRACK          : '[';
RBRACK          : ']';
SEMI            : ';';
COMMA           : ',';
DOT             : '.';
SAFE_DOT        : '?.';
NULL_COALESCE   : '??';
ASSIGN          : '=';
GT              : '>';
LT              : '<';
BANG            : '!';
TILDE           : '~';
QUES            : '?';
COLON           : ':';
EQUAL           : '==';
EXACT_EQUAL     : '===';
LE              : '<=';
GE              : '>=';
NOTEQUAL        : '!=';
EXACT_NOTEQUAL  : '!==';
ALT_NOTEQUAL    : '<>';
ANDAND          : '&&';
OROR            : '||';
INC             : '++';
DEC             : '--';
ADD             : '+';
SUB             : '-';
MUL             : '*';
DIV             : '/';
BITAND          : '&';
BITOR           : '|';
CARET           : '^';
MOD             : '%';
LSHIFT          : '<<';
RSHIFT          : '>>';
URSHIFT         : '>>>';
ADD_ASSIGN      : '+=';
SUB_ASSIGN      : '-=';
MUL_ASSIGN      : '*=';
DIV_ASSIGN      : '/=';
AND_ASSIGN      : '&=';
OR_ASSIGN       : '|=';
XOR_ASSIGN      : '^=';
MOD_ASSIGN      : '%=';
LSHIFT_ASSIGN   : '<<=';
RSHIFT_ASSIGN   : '>>=';
URSHIFT_ASSIGN  : '>>>=';
ARROW           : '=>';
ANNOTATION_AT   : '@';

// Identifiers
IDENTIFIER      : [a-zA-Z_] [a-zA-Z0-9_]*;

// Number Literals
INTEGER_LITERAL : [0-9]+ [lL]?;
DECIMAL_LITERAL : [0-9]+ '.' [0-9]+ ([eE] [+-]? [0-9]+)? [dDmM]?
                | '.' [0-9]+ ([eE] [+-]? [0-9]+)? [dDmM]?
                | [0-9]+ [eE] [+-]? [0-9]+ [dDmM]?;

// String Literals (Apex uses single quotes, with \' escaping)
STRING_LITERAL  : '\'' ( '\\\'' | '\\\\' | ~['\\] )* '\'';

// Comments & Whitespace
WS              : [ \t\r\n]+ -> skip;
LINE_COMMENT    : '//' ~[\r\n]* -> skip;
BLOCK_COMMENT   : '/*' .*? '*/' -> skip;
