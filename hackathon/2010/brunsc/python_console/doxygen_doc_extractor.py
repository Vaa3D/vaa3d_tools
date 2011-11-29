"""
Fixed and improved version based on "extracting from C++ doxygen documented file Author G.D." and py++ code.

Distributed under the Boost Software License, Version 1.0. (See
accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)

Extensively modified by C.M. Bruns April 2010.
"""

import re
from pygccxml import declarations
import unittest

class doxygen_doc_extractor:
    """
    Extracts Doxygen styled documentation from source or generates it from description.
    """
    def __init__(self):
        #for caching source
        self.file_name = None
        self.source = None
    #__init__

    def __call__(self, declaration):
        if self.file_name != declaration.location.file_name:
            self.file_name = declaration.location.file_name
            self.source = open(declaration.location.file_name).readlines()
            self.declaration = declaration

        find_block_end = False
        doc_lines = []
        
        # First look for a same-line documentation:
        same_line_index = declaration.location.line - 1 # off by one issue
        line = self.source[same_line_index]
        # print declaration, same_line_index, line
        doc = extract_same_line_doc(line)
        doc = clear_str(doc)
        # print declaration, line
        if doc:
            # print """found same line documentation:
            #     %s
            #     '%s'""" % (line, doc)
            if doc:
                doc_lines.insert(0, doc)
                # return self.finalize_doc_lines(doc_lines)
        # Next look for a doxygen comment above the declaration
        # Is there a pure comment line directly above the declaration?
        line_index = declaration.location.line - 2
        # March backwards in the file looking for comments, avoiding other declarations
        while (line_index >= 0):
            line = self.source[line_index]
            if might_be_doxygen_comment_end_line(line):
                break
            if line.find(';') >= 0: # another declaration
                break
            line_index -= 1
        if (line_index < 0):
            return self.finalize_doc_lines(doc_lines) # Only blanks above declaration
        line = self.source[line_index]
        # print "investigating '%s'" % line
        # print line_index
        if not might_be_doxygen_comment_end_line(line):
            # print "not end comment"
            return self.finalize_doc_lines(doc_lines) # no doxygen comment here
        # We need to know whether we are within a C-style comment "... */"
        doc_lines.extend(self.grab_doxygen_comment(line_index))
        return self.finalize_doc_lines(doc_lines)
    #__call__()

    def grab_doxygen_comment(self, line_index):
        """Returns a list of doxygen comments ending at line line_index"""
        doc_lines = []
        if line_index < 0:
            return doc_lines
        line = self.source[line_index]
        # "///" lines are definitely doxygen comments
        # print "checking line '%s'" % line
        if is_cpp_comment_line(line):
            # print "is cpp comment"
            doc_lines.insert(0, clear_str(line))
            # print "cpp comment", doc_lines
            doc_lines = self.grab_doxygen_comment(line_index - 1) + doc_lines
        elif is_c_end_comment(line):
            # print "is c end comment"
            doc_lines = self.grab_c_comment(line_index) + doc_lines
        elif is_oneline_c_comment(line):
            # print "is oneline c comment"
            doc_lines.insert(0, clear_str(line))
            doc_lines = self.grab_doxygen_comment(line_index - 1) + doc_lines
            # print "one line C comment", doc_lines
        return doc_lines

    def grab_c_comment(self, line_index):
        """Returns a multiline c-style doxygen comment ending at line line_index"""
        doc_lines = []
        doc_lines.append(clear_str(self.source[line_index])) # final line of comment
        while True:
            line_index -= 1
            if line_index < 0:
                raise IndexError('End comment without start')
                break
            line = self.source[line_index]
            doc_lines.insert(0, clear_str(line))
            if is_c_start_comment(line):
                break
            if is_c_nondoxygen_start_comment(line):
                return []
        # print "C comment", doc_lines
        more_comment = self.grab_doxygen_comment(line_index - 1)
        # print "more comment = ", more_comment, self.source[line_index - 1]
        return self.grab_doxygen_comment(line_index - 1) + doc_lines
        

    def is_code(self, tmp_str):
        """
        Detects if tmp_str is code or not
        """
        try:
            beg = tmp_str.lstrip()[:2]
            return beg != "//" and beg != "/*"
        except:
            pass
        return False
    #is_code()

    def finalize_doc_lines(self, doc_lines):
        if not doc_lines:
            return None
        final_doc_lines = [ line.replace("\n","\\n") for line in doc_lines[:-1] ]
        final_doc_lines.append(doc_lines[-1].replace("\n",""))
        # remove blank lines at the beginning and end
        while final_doc_lines and final_doc_lines[0] == "":
            final_doc_lines = final_doc_lines[1:]
        while final_doc_lines and final_doc_lines[-1] == "":
            final_doc_lines = final_doc_lines[:-1]
        # class docstrings should start and end with a blank line
        # http://www.python.org/dev/peps/pep-0257/
        if declarations.is_class(self.declaration):
            # print "is class", self.declaration
            final_doc_lines.append("")
            final_doc_lines.insert(0, "")
            pass
        return '\"' + '\\n'.join(final_doc_lines) + '\"'    

def clear_str(tmp_str):
    """
    Replace */! by space and \brief, @fn, \param, etc
    """
    if not tmp_str:
        return None
    
    # Keep indentation on left, but only after comment characters
    # Remove initial spaces followed by comment characters of any kind
    tmp_str = re.sub(r'^\s*[/*!]+', '', tmp_str)
    # Remove final spaces and comment characters
    tmp_str = re.sub(r'[/*]+$', '', tmp_str)
    tmp_str = re.sub(r'\s+$', '', tmp_str)
    
    # Remove "@brief" statements
    tmp_str = re.sub(r'^(\s*)[\\@]brief\s?', r'\1', tmp_str)

    tmp_str = reduce(clean, [tmp_str, "\\fn","@fn","\\ref","@ref", "\"", "\'", "\\c"])
    
    # Transform param statements to epydoc format
    tmp_str = re.sub(r'[@\\]param\s+([^ :]+):?', r'@param \1:', tmp_str)

    # Transform \p doxyygen parameter references to epydoc C{} code font
    tmp_str = re.sub(r'\\[apc]\s(\S+)', r'C{\1}', tmp_str)

    # Convert html lists to epydoc lists
    tmp_str = re.sub(r'<(li|LI)>', '  - ', tmp_str)
    tmp_str = re.sub(r'</(li|LI)>', '', tmp_str)
    tmp_str = re.sub(r'</?(ul|UL|ol|OL)>', '', tmp_str)

    # Convert doxygen \par headings to epydoc sections
    m = re.match(r'^(\s*)\\par\s+(\S?.*\S)\s*$', tmp_str)
    if m:
        tmp_str = (m.group(1) + m.group(2) + "\n" +    # Theory:
                   m.group(1) + len(m.group(2)) * "-") # -------

    # Smooth over unicode conversion errors here, rather than waiting
    # for trouble at Py++ source code generation time.
    tmp_str = unicode(tmp_str, errors='ignore')

    #commands list taken form : http://www.stack.nl/~dimitri/doxygen/commands.html
    replacement_list = [
#               "a",
        "addindex",
        "addtogroup",
        "anchor",
        "arg",
        "attention",
        "author",
#               "b",
#               "brief",
        "bug",
#               "c",
        "callgraph",
        "callergraph",
        "category",
        "class",
        ("code","[Code]"),
        "cond",
        "copybrief",
        "copydetails",
        "copydoc",
        "date",
        "def",
        "defgroup",
        "deprecated",
        "details",
        "dir",
        "dontinclude",
        ("dot","[Dot]"),
        "dotfile",
        "e",
        "else",
        "elseif",
        "em",
        ("endcode","[/Code]"),
        "endcond",
        ("enddot","[/Dot]"),
        "endhtmlonly",
        "endif",
        "endlatexonly",
        "endlink",
        "endmanonly",
        "endmsc",
        "endverbatim",
        "endxmlonly",
        "enum",
        "example",
        "exception",
        "extends",
        "f$",
        "f[",
        "f]",
        "f{",
        "f}",
        "file",
#               "fn",
        "headerfile",
        "hideinitializer",
        "htmlinclude",
        "htmlonly",
        "if",
        "ifnot",
        "image",
        "implements",
        "include",
        "includelineno",
        "ingroup",
        "internal",
        "invariant",
        "interface",
        "latexonly",
        "li",
        "line",
        "link",
        "mainpage",
        "manonly",
        "memberof",
        "msc",
#               "n",
        "name",
        "namespace",
        "nosubgrouping",
        "note",
        "overload",
#               "p",
        "package",
        "page",
#        "par",
        "paragraph",
#        "param",
        "post",
        "pre",
#               "private (PHP only)",
#               "privatesection (PHP only)",
        "property",
#               "protected (PHP only)",
#               "protectedsection (PHP only)",
        "protocol",
#               "public (PHP only)",
#               "publicsection (PHP only)",
#               "ref",
        "relates",
        "relatesalso",
        "remarks",
        "return",
        "retval",
        "sa",
        "section",
        "see",
        "showinitializer",
        "since",
        "skip",
        "skipline",
        "struct",
        "subpage",
        "subsection",
        "subsubsection",
        "test",
        "throw",
        ("todo","TODO"),
        "tparam",
        "typedef",
        "union",
        "until",
        "var",
        "verbatim",
        "verbinclude",
        "version",
        "warning",
        "weakgroup",
        "xmlonly",
        "xrefitem",
#               "$",
#               "@",
#               "\",
#               "&",
#               "~",
#               "<",
#               ">",
#               "#",
#               "%",
        ]

    for command in replacement_list:
        try:
            old,new = command
        except ValueError:
            old = command
            new = command.capitalize()+":"
        tmp_str = clean(tmp_str, "@"+old, new)
        tmp_str = clean(tmp_str, "\\"+old, new)

    # Replace any remaining unescaped backslashes with an escaped backslash
    tmp_str = re.sub(r'([^\\])\\([^\\])', r'\1\\\\\2', tmp_str)

    return tmp_str
#clean_str()

#class doxygen_doc_extractor# Category 3 of doxygen comments, (only) on the same line as item being documented
# e.g. int foo; //!< foo is an integer...
# can start with "//!", "///" or "/**"
dox3a_re = re.compile(r'//[!/]<(.*)') # "//!" or "///"
dox3b_re = re.compile(r'/\*\*<(.*)\*/') # "/**<", ended by "*/"
def extract_same_line_doc(line):
    m = dox3a_re.search(line)
    if m:
        return m.group(1)
    m = dox3b_re.search(line)
    if m:
        return m.group(1)
    return None
    
dox_end1_re = re.compile(r'^\s*///.*$') # anything starting with "///"
dox_end2_re = re.compile(r'^\s*/\*[!*].*\*/\s*$') # One line comment "/** whatever */"
dox_end3_re = re.compile(r'.*\*/\s*$') # end of multiline doxygen comment "whatever */"
one_line_comment_re = re.compile(r'^.*/\*.*\*/.*$') # anything with a one line "/* whatever */" comment
def might_be_doxygen_comment_end_line(line):
    if dox_end1_re.match(line):
        return True
    if dox_end2_re.match(line):
        return True
    if dox_end3_re.match(line):
        if not one_line_comment_re.match(line):
            return True
    return False

def is_blank_line(line):
    if line.strip() == "":
        return True
    return False

def clean(str, old, new=""):
    "This is the best I could figure for missing clean() function --cmb"
    if not str:
        return ""
    return str.replace(old, new)
    
def is_cpp_comment_line(line):
    "whether line is a c++ style doxygen comment"
    if line.lstrip().startswith("///"):
        return True
    if line.lstrip().startswith("//!"):
        return True
    return False

def is_c_end_comment(line):
    "Line ends, but does not begin, a c-style comment"
    if not line.rstrip().endswith("*/"):
        return False
    if line.find("/*") >= 0:
        return False
    return True
    
def is_oneline_c_comment(line):
    if not line.rstrip().endswith("*/"):
        return False
    if line.lstrip().startswith("/**"):
        return True
    if line.lstrip().startswith("/*!"):
        return True
    return False

def is_c_start_comment(line):
    if line.rstrip().endswith("*/"):
        return False # want start only, with no end
    if line.lstrip().startswith("/**"):
        return True
    if line.lstrip().startswith("/*!"):
        return True
    return False

def is_c_nondoxygen_start_comment(line):
    if line.lstrip().startswith("/*"):
        return True
    return False

class TestDocExtractor(unittest.TestCase):
    def test_clear(self):
        self.assertEqual(clear_str(r"  /*   f"), "   f")
        self.assertEqual(clear_str(r"   f"), "   f")
        self.assertEqual(clear_str(r"   f */"), "   f")
        self.assertEqual(clear_str(r"   \brief  f "), "    f")
        self.assertEqual(
            clear_str(r"  @param foo desc f"),
            r"  @param foo: desc f") # epydoc format
        self.assertEqual(
            clear_str(r"  \param foo desc f"),
            r"  @param foo: desc f") # epydoc format
        self.assertEqual(
            clear_str(r"  @param foo: desc f"),
            r"  @param foo: desc f") # epydoc input should remain unchanged
        self.assertEqual(
            clear_str(r"  the \p foo parameter"),
            r"  the L{foo} parameter")
        self.assertEqual( # clear trailing newlines
            clear_str("  foo \n"),
            r"  foo")
        # convert html list to epydoc list
        self.assertEqual(
            clear_str("  <li>foo</li>"),
            r"    - foo")
        # convert doxygen \par to epydoc sections
        self.assertEqual(
            clear_str(r" \par Theory:"),
            " Theory:\n -------")        
        
    def test_finalize(self):
        ex = doxygen_doc_extractor()
        ex.declaration = ""
        self.assertEqual(ex.finalize_doc_lines(["f"]), '"f"')
        self.assertEqual(ex.finalize_doc_lines(["", "f", ""]), '"f"')
        self.assertEqual(ex.finalize_doc_lines(
            ["", "f", "g", ""]), 
            r'"f\ng"')
        self.assertEqual(ex.finalize_doc_lines(
            ["", "f", "  g", ""]), 
            r'"f\n  g"')

if __name__ == "__main__":
    unittest.main()
