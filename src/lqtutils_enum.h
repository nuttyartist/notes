/**
 * MIT License
 *
 * Copyright (c) 2020-2021 Luca Carlon
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/

// Credit: https://github.com/carlonluca/lqtutils

#ifndef LQTUTILS_ENUM_H
#define LQTUTILS_ENUM_H

#include <QObject>
#include <QQmlEngine>

#define L_DECLARE_ENUM(enumName, ...)                                                      \
  namespace enumName {                                                                     \
  Q_NAMESPACE                                                                              \
  enum Value { __VA_ARGS__ };                                                              \
  Q_ENUM_NS(Value)                                                                         \
  inline int qmlRegister##enumName(const char *uri, int major, int minor)                  \
  {                                                                                        \
    return qmlRegisterUncreatableMetaObject(enumName::staticMetaObject, uri, major, minor, \
                                            #enumName, "Access to enums & flags only");    \
  }                                                                                        \
  inline int qRegisterMetaType()                                                           \
  {                                                                                        \
    return ::qRegisterMetaType<enumName::Value>(#enumName);                                \
  }                                                                                        \
  inline void registerEnum(const char *uri, int major, int minor)                          \
  {                                                                                        \
    enumName::qmlRegister##enumName(uri, major, minor);                                    \
    enumName::qRegisterMetaType();                                                         \
  }                                                                                        \
  }

#endif // LQTUTILS_ENUM_H
