//
// Created by cout970 on 27/10/16.
//

#ifndef COMPUTER_CTYPE_H
#define COMPUTER_CTYPE_H

int isalnum(int c);

int isalpha(int c);

int iscntrl(int c);

int isdigit(int c);

int isgraph(int c);

int islower(int c);

int isprint(int c);

int ispunct(int c);

int isspace(int c);

int isupper(int c);

int isxdigit(int c);

int tolower(int c);

int toupper(int c);

int isascii(int c);

/*
 *
1	Digits
This is a set of whole numbers { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }.

2	Hexadecimal digits
This is the set of { 0 1 2 3 4 5 6 7 8 9 A B C D E F a b c d e f }.

3	Lowercase letters
This is a set of lowercase letters { a b c d e f g h i j k l m n o p q r s t u v w x y z }.

4	Uppercase letters
This is a set of uppercase letters {A B C D E F G H I J K L M N O P Q R S T U V W X Y Z }.

5	Letters
This is a set of lowercase and uppercase letters.

6	Alphanumeric characters
This is a set of Digits, Lowercase letters and Uppercase letters.

7	Punctuation characters
This is a set of ! " # $ % & ' ( ) * + , - . / : ; < = > ? @ [ \ ] ^ _ ` { | } ~

8	Graphical characters
This is a set of Alphanumeric characters and Punctuation characters.

9	Space characters
This is a set of tab, newline, vertical tab, form feed, carriage return, and space.

10	Printable characters
This is a set of Alphanumeric characters, Punctuation characters and Space characters.

11	Control characters
In ASCII, these characters have octal codes 000 through 037, and 177 (DEL).

12	Blank characters
These are spaces and tabs.

13	Alphabetic characters
This is a set of Lowercase letters and Uppercase letters.

 */

#endif //COMPUTER_CTYPE_H
