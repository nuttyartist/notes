# Style and format recursively
astyle --pad-oper --pad-first-paren-out --align-pointer=type --remove-brackets --convert-tabs --max-code-length=80 --style=google --lineend=windows --suffix=none --recursive ../../*.h ../../*.cpp ../../*.c
