Introduction
============

In log4cxx 0.10, you have to know the macros are "blocks" not "statements" as any
conditional use of a logging macro without enclosing brackets
has to be written without a trailing semicolon.

For example:

```c++
if (someCondition)
  LOG4CXX_INFO(m_log, "Condition 1 message") // Note a semicolon here will cause a compile error
else
  LOG4CXX_INFO(m_log, "Alternate message)
```

As a result of knowing the macros are "blocks", most LOG4CXX_ XXX() code
often does not have a trailing semicolon.

In log4cxx 0.11 macros are statements. That is, you add a semicolon after the macro usage.

log4cxx_10_to_11 is a utility that provides options for checking log4cxx macro usage.
By default it lists the files in the provided directories that have will not compile
with log4cxx 0.11.

Synopsis
========

log4cxx_10_to_11 {options} {file_or_directory_list}

Option             | Description
-------------------|------------------------------------------------
-h [ --help ]      |   produce help message
-q [ --quiet ]     |   do not print file names
-v [ --verbose ]   |   list the number of macros to be adjusted in each file
--only_11          |   modify files to work with 0.11
--both_10_and_11   |   modify files to work with 0.10 and 0.11
-e [ --ext ] arg   |   add to the list of checked file extensions: default [.cpp, .cxx, .hpp, .h]

Use --only_11 to change to a syntax that will not need to compile with log4cxx 0.10.
It will change the above example to:

```c++
if (someCondition)
  LOG4CXX_INFO(m_log, "Condition 1 message"); // Note a semicolon here will cause a compile error
else
  LOG4CXX_INFO(m_log, "Alternate message);
```

To be able to compile your systems with both 0.10 and 0.11 (e.g. for a
transitional period or to avoid a 'big bang' switch over) use
--both_10_and_11. It will change the above example to:

```c++
if (someCondition)
{
  LOG4CXX_INFO(m_log, "Condition 1 message"); // Note a semicolon here will cause a compile error
}
else
  LOG4CXX_INFO(m_log, "Alternate message);
```
