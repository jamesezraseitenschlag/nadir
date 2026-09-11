// ApexParser.g4
// Sourced and adapted from antlr/grammars-v4, Salesforce apex-jorje, and PMD Apex AST.
// Standing on the shoulders of giants (or at least people who actually enjoy writing BNF rules).

parser grammar ApexParser;

options {
    tokenVocab = ApexLexer;
}

compilationUnit
    : (typeDeclaration | statement)* EOF
    ;

typeDeclaration
    : modifier* (classDeclaration | interfaceDeclaration | enumDeclaration)
    ;

classDeclaration
    : CLASS IDENTIFIER (EXTENDS typeRef)? (IMPLEMENTS typeList)? classBody
    ;

interfaceDeclaration
    : INTERFACE IDENTIFIER (EXTENDS typeList)? interfaceBody
    ;

enumDeclaration
    : ENUM IDENTIFIER LBRACE (IDENTIFIER (COMMA IDENTIFIER)*)? RBRACE
    ;

classBody
    : LBRACE classBodyDeclaration* RBRACE
    ;

interfaceBody
    : LBRACE interfaceMethodDeclaration* RBRACE
    ;

classBodyDeclaration
    : semiStatement
    | modifier* (constructorDeclaration | methodDeclaration | fieldDeclaration | propertyDeclaration | classDeclaration | interfaceDeclaration)
    | staticBlock
    ;

staticBlock
    : STATIC block
    ;

constructorDeclaration
    : IDENTIFIER formalParameters (THROW qualifiedNameList)? block
    ;

methodDeclaration
    : (typeRef | VOID) IDENTIFIER formalParameters (THROW qualifiedNameList)? (block | SEMI)
    ;

interfaceMethodDeclaration
    : modifier* (typeRef | VOID) IDENTIFIER formalParameters (THROW qualifiedNameList)? SEMI
    ;

fieldDeclaration
    : typeRef variableDeclarators SEMI
    ;

propertyDeclaration
    : typeRef IDENTIFIER LBRACE propertyBlock+ RBRACE
    ;

propertyBlock
    : modifier* (GET | SET) (SEMI | block)
    ;

GET : 'get';
SET : 'set';

variableDeclarators
    : variableDeclarator (COMMA variableDeclarator)*
    ;

variableDeclarator
    : IDENTIFIER (ASSIGN expression)?
    ;

formalParameters
    : LPAREN (formalParameter (COMMA formalParameter)*)? RPAREN
    ;

formalParameter
    : modifier* typeRef IDENTIFIER
    ;

modifier
    : GLOBAL
    | PUBLIC
    | PROTECTED
    | PRIVATE
    | TRANSIENT
    | STATIC
    | FINAL
    | ABSTRACT
    | VIRTUAL
    | OVERRIDE
    | WITH_SHARING
    | WITHOUT_SHARING
    | INHERITED_SHARING
    | annotation
    ;

annotation
    : AT IDENTIFIER (LPAREN (elementValuePairs | elementValue)? RPAREN)?
    ;

elementValuePairs
    : elementValuePair (COMMA elementValuePair)*
    ;

elementValuePair
    : IDENTIFIER ASSIGN elementValue
    ;

elementValue
    : expression
    ;

typeRef
    : typeName (typeArguments)? (LBRACK RBRACK)*
    ;

typeName
    : BLOB | BOOLEAN | DATE | DATETIME | DECIMAL | DOUBLE | ID_TYPE | INTEGER | LONG | OBJECT | STRING | TIME | LIST | SET | MAP | qualifiedName
    ;

typeArguments
    : LT typeRef (COMMA typeRef)* GT
    ;

typeList
    : typeRef (COMMA typeRef)*
    ;

qualifiedNameList
    : qualifiedName (COMMA qualifiedName)*
    ;

qualifiedName
    : IDENTIFIER (DOT IDENTIFIER)*
    ;

block
    : LBRACE statement* RBRACE
    ;

statement
    : block
    | IF LPAREN expression RPAREN statement (ELSE statement)?
    | FOR LPAREN forControl RPAREN statement
    | WHILE LPAREN expression RPAREN statement
    | DO statement WHILE LPAREN expression RPAREN SEMI
    | TRY block catchClause* (FINALLY block)?
    | RETURN expression? SEMI
    | THROW expression SEMI
    | BREAK SEMI
    | CONTINUE SEMI
    | dmlStatement
    | typeRef variableDeclarators SEMI
    | expressionStatement
    | SEMI
    ;

catchClause
    : CATCH LPAREN qualifiedName IDENTIFIER RPAREN block
    ;

forControl
    : forInit? SEMI expression? SEMI expressionList?
    | typeRef IDENTIFIER COLON expression
    ;

forInit
    : typeRef variableDeclarators
    | expressionList
    ;

dmlStatement
    : (INSERT | UPDATE | UPSERT | DELETE | UNDELETE) expression SEMI
    ;

expressionStatement
    : expression SEMI
    ;

semiStatement
    : SEMI
    ;

expressionList
    : expression (COMMA expression)*
    ;

expression
    : assignmentExpression
    ;

assignmentExpression
    : conditionalExpression ((ASSIGN | ADD_ASSIGN | SUB_ASSIGN | MUL_ASSIGN | DIV_ASSIGN | AND_ASSIGN | OR_ASSIGN | XOR_ASSIGN) expression)?
    ;

conditionalExpression
    : logicalOrExpression (QUES expression COLON expression)?
    ;

logicalOrExpression
    : logicalAndExpression (OROR logicalAndExpression)*
    ;

logicalAndExpression
    : equalityExpression (ANDAND equalityExpression)*
    ;

equalityExpression
    : relationalExpression ((EQUAL | NOTEQUAL | EXACT_EQUAL | EXACT_NOTEQUAL | ALT_NOTEQUAL) relationalExpression)*
    ;

relationalExpression
    : additiveExpression ((LT | GT | LE | GE | INSTANCEOF) additiveExpression)*
    ;

INSTANCEOF : 'instanceof';

additiveExpression
    : multiplicativeExpression ((ADD | SUB) multiplicativeExpression)*
    ;

multiplicativeExpression
    : unaryExpression ((MUL | DIV | MOD) unaryExpression)*
    ;

unaryExpression
    : (ADD | SUB | INC | DEC | BANG | TILDE) unaryExpression
    | primary (INC | DEC)?
    ;

primary
    : literal
    | qualifiedName
    | LPAREN expression RPAREN
    | newExpression
    | methodCall
    | fieldOrPropertyAccess
    | arrayAccess
    | soqlQuery
    ;

methodCall
    : (primary (DOT | SAFE_DOT))? IDENTIFIER LPAREN expressionList? RPAREN
    ;

fieldOrPropertyAccess
    : primary (DOT | SAFE_DOT) IDENTIFIER
    ;

arrayAccess
    : primary LBRACK expression RBRACK
    ;

newExpression
    : 'new' typeRef (arguments | listInitializer | mapInitializer)
    ;

arguments
    : LPAREN expressionList? RPAREN
    ;

listInitializer
    : LBRACE expressionList? RBRACE
    ;

mapInitializer
    : LBRACE (mapEntry (COMMA mapEntry)*)? RBRACE
    ;

mapEntry
    : expression ARROW expression
    ;

soqlQuery
    : LBRACK SELECT selectList FROM IDENTIFIER (WHERE whereClause)? (ORDER BY orderByClause)? (LIMIT INTEGER_LITERAL)? RBRACK
    ;

selectList
    : qualifiedName (COMMA qualifiedName)*
    ;

whereClause
    : whereCondition (AND whereCondition | OR whereCondition)*
    ;

whereCondition
    : qualifiedName (EQUAL | NOTEQUAL | LT | GT | LE | GE | LIKE | IN) (literal | COLON IDENTIFIER)
    ;

orderByClause
    : qualifiedName (ASC | DESC)?
    ;

literal
    : INTEGER_LITERAL
    | DECIMAL_LITERAL
    | STRING_LITERAL
    | BOOLEAN_LITERAL
    | NULL_LITERAL
    ;
