/********************************************************************************************
Parser descendente recursivo LL(1) para Expresiones Regulares
Gramática (con Sigma={a,b,c}):

E  -> T E'
E' -> '|' T E' 
E' -> $               // $ representa a lambda
T  -> F T' 
T' -> '.' F T' 
T' -> $ 
F  -> P F' 
F' -> '*' F' 
F' -> $
P  -> '(' E ')' 
P  -> 'a' 
P  -> 'b'
P  -> 'c'


Ejemplos expresiones regulares válidas   |         Parsing tree                       
a                                        |  a                                  | 
b                                        |    b                              /   \
c                                        |      c                           .     .
a*                                       |         *                       / \   / \
(a)*                                     |         |   *                  *   c *   |
a.b                                      |         a   |     .            |     |  / \ 
a|b                                      |             a    / \    |      |     a *   *  
((a.c|b)*.c)|(a*.(b*|c*))                |                 a   b  / \    / \      |   | 
                                                                 a   b  .   b     a   b
                                                                       / \
                                                                      a   c

*********************************************************************************************/

#include <stdio.h>
#include <string.h>

#define SUCCESS 1
#define FAILED 0

// Function prototypes
int E(), Edash(), T(), Tdash(), F(), Fdash(), P();

const char *cursor;
char string[64];

int main(){
    puts("Enter the Regular Expression");
    scanf("%s", string); // Read input from the user
    cursor = string;
    puts("");
    puts("Input          Action");
    puts("--------------------------------");

    // Call the starting non-terminal E
    if (E() && *cursor == '\0'){ // If parsing is successful and the cursor has reached the end
        puts("--------------------------------");
        puts("Regular Expression is successfully parsed");
        return 0;
    }else{
        puts("--------------------------------");
        puts("Error in parsing Regular Expression");
        return 1;
    }
}

// Grammar rule: E -> T E'
int E(){
   switch (*cursor) {
       case 'a': // Simbolos del alfabeto de la expresion regular {a,b,c}
       case 'b':
       case 'c':
       case '(':	
              printf("%-16s E -> T E'\n", cursor);

              if (T()){ // Call non-terminal T
                  if (Edash()){ // Call non-terminal E'
                      return SUCCESS;
                  }else{
                      return FAILED;
                  }
              }else{
                  return FAILED;
              }
       default:
            // Código a ejecutar si no coincide con ningún caso
            return FAILED;
   }       
}

// Grammar rule: E' -> | T E' | $
int Edash(){
   switch (*cursor) {
       case '|':	
              printf("%-16s E' -> | T E'\n", cursor);
              cursor++;

              if (T()){ // Call non-terminal T
                  if (Edash()){ // Call non-terminal E'
                      return SUCCESS;
                  }else{
                      return FAILED;
                  }
              }else{
                  return FAILED;
              }
      case ')':      
      case '\0': 
             printf("%-16s E' -> $\n", cursor);
             return SUCCESS;
      default:
            // Código a ejecutar si no coincide con ningún caso
            return FAILED;  
    }
}

// Grammar rule: T -> F T' 
int T(){
   switch (*cursor) {
       case 'a': // Simbolos del alfabeto de la expresion regular {a,b,c}
       case 'b':
       case 'c':
       case '(':	
              printf("%-16s T -> F T'\n", cursor);

              if (F()){ // Call non-terminal F
                 if (Tdash()){ // Call non-terminal T'
                    return SUCCESS;
                 }else{
                    return FAILED;
                 }
              }else{
                 return FAILED;
              }
       default:
            // Código a ejecutar si no coincide con ningún caso
            return FAILED;
   }       
}

// Grammar rule: T' -> . F T' | $
int Tdash(){
   switch (*cursor) {
       case '.':
              printf("%-16s T' -> . F T'\n", cursor);
              cursor++;

              if (F()){ // Call non-terminal F
                  if (Tdash()){ // Call non-terminal T'
                      return SUCCESS;
                  }else{
                      return FAILED;
                  }
              }else{
                  return FAILED;
              }
      case '|':
      case ')':      
      case '\0': 
             printf("%-16s T' -> $\n", cursor);
             return SUCCESS;
      default:
            // Código a ejecutar si no coincide con ningún caso
            return FAILED;  
    }
}

// Grammar rule: F -> P F' 
int F(){
   switch (*cursor) {
       case 'a': // Simbolos del alfabeto de la expresion regular {a,b,c}
       case 'b':
       case 'c':
       case '(':
              printf("%-16s F -> P F'\n", cursor);

              if (P()){ // Call non-terminal P
                  if (Fdash()){ // Call non-terminal F'
                      return SUCCESS;
                  }else{
                      return FAILED;
                  }
             }else{
                  return FAILED;
             }
       default:
            // Código a ejecutar si no coincide con ningún caso
            return FAILED;
   }
}   

// Grammar rule: F' -> * F' | $
int Fdash(){
   switch (*cursor) {
      case '*':
             printf("%-16s F' -> * F' \n", cursor);
             cursor++;

             if (Fdash()){ // Call non-terminal Fdash
                return SUCCESS;
             }else{
                return FAILED;
             }
      case '|':
      case '.': 
      case ')':      
      case '\0': 
             printf("%-16s F' -> $\n", cursor);
             return SUCCESS;
      default:
            // Código a ejecutar si no coincide con ningún caso
            return FAILED;  
    }
}

// Grammar rule: P -> ( E ) | a | b | c
int P(){
   switch (*cursor) {
      case '(':
             printf("%-16s P -> ( E )\n", cursor);
             cursor++; // Avanza '(' en la entrada

             if (E()){ // Call non-terminal E
                if (*cursor == ')'){
                   cursor++;
                   return SUCCESS;
                }else{
                   return FAILED;
                }
             }else{
                return FAILED;
             }
       case 'a': // Simbolos del alfabeto de la expresion regular {a,b,c}
       case 'b':
       case 'c':
             printf("%-16s P -> %c\n", cursor,*cursor);
             cursor++;
             return SUCCESS;
       default:
            // Código a ejecutar si no coincide con ningún caso
            return FAILED;
    }
}
