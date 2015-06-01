#region License
//
// Command Line Library: HelpTextFixture.cs
//
// Author:
//   Giacomo Stelluti Scala (gsscoder@ymail.com)
// Contributor(s):
//   Steven Evans
// 
// Copyright (C) 2005 - 2010 Giacomo Stelluti Scala
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#endregion
#if UNIT_TESTS
#region Using Directives
using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;
#endregion

namespace CommandLine.Text.Tests
{
    [TestFixture]
    public sealed class HelpTextFixture
    {
        class MockOptions
        {
            [Option("v", "verbose")]
            public bool Verbose = false;

            [Option(null, "input-file")]
            public string FileName = string.Empty;
        }

        private class MockOptionsWithLongDescription
        {
            [Option("v", "verbose", HelpText = "This is the description of the verbosity to test out the wrapping capabilities of the Help Text.")]
            public bool Verbose = false;

            [Option(null, "input-file", HelpText = "This is a very long description of the Input File argument that gets passed in.  It should  be passed in as a string.")]
            public string FileName = string.Empty;
        }

        private class MockOptionsWithLongDescriptionAndNoSpaces
        {
            [Option("v", "verbose", HelpText = "Before 012345678901234567890123 After")]
            public bool Verbose = false;

            [Option(null, "input-file", HelpText = "Before 012345678901234567890123456789 After")]
            public string FileName = string.Empty;
        }

        private HelpText _helpText;

        [SetUp]
        public void SetUp()
        {
            _helpText = new HelpText(new HeadingInfo(ThisAssembly.Title, ThisAssembly.Version));
        }

        [Test]
        public void AddAnEmptyPreOptionsLineIsAllowed()
        {
            _helpText.AddPreOptionsLine(string.Empty); // == ""
        }

        /// <summary>
        /// Ref.: #REQ0002
        /// </summary>
        [Test]
        public void PostOptionsLinesFeatureAdded()
        {
            var local = new HelpText("Heading Info.");
            local.AddPreOptionsLine("This is a first pre-options line.");
            local.AddPreOptionsLine("This is a second pre-options line.");
            local.AddOptions(new MockOptions());
            local.AddPostOptionsLine("This is a first post-options line.");
            local.AddPostOptionsLine("This is a second post-options line.");

            string help = local.ToString();

            string[] lines = help.Split(new string[] { Environment.NewLine }, StringSplitOptions.None);
            Assert.AreEqual(lines[lines.Length - 2], "This is a first post-options line.");
            Assert.AreEqual(lines[lines.Length - 1], "This is a second post-options line.");
        }

        [Test]
        public void WhenHelpTextIsLongerThanWidthItWillWrapAroundAsIfInAColumn()
        {
            _helpText.MaximumDisplayWidth = 40;
            _helpText.AddOptions(new MockOptionsWithLongDescription());
            string help = _helpText.ToString();

            string[] lines = help.Split(new[] {Environment.NewLine}, StringSplitOptions.None);
            Assert.AreEqual(lines[2], "  v, verbose    This is the description", "The first line should have the arguments and the start of the Help Text.");
            string formattingMessage = "Beyond the second line should be formatted as though it's in a column.";
            Assert.AreEqual(lines[3], "                of the verbosity to ", formattingMessage);
            Assert.AreEqual(lines[4], "                test out the wrapping ", formattingMessage);
            Assert.AreEqual(lines[5], "                capabilities of the ", formattingMessage);
            Assert.AreEqual(lines[6], "                Help Text.", formattingMessage);
        }

        [Test]
        public void LongHelpTextWithoutSpaces()
        {
            _helpText.MaximumDisplayWidth = 40;
            _helpText.AddOptions(new MockOptionsWithLongDescriptionAndNoSpaces());
            string help = _helpText.ToString();

            string[] lines = help.Split(new[] { Environment.NewLine }, StringSplitOptions.None);
            Assert.AreEqual("  v, verbose    Before ", lines[2]);
            Assert.AreEqual("                012345678901234567890123", lines[3]);
            Assert.AreEqual("                After", lines[4]);
            Assert.AreEqual("  input-file    Before ", lines[5]);
            Assert.AreEqual("                012345678901234567890123", lines[6]);
            Assert.AreEqual("                456789 After", lines[7]);
        }

        [Test]
        public void LongPreAndPostLinesWithoutSpaces()
        {
            var local = new HelpText("Heading Info.");
            local.MaximumDisplayWidth = 40;
            local.AddPreOptionsLine("Before 0123456789012345678901234567890123456789012 After");
            local.AddOptions(new MockOptions());
            local.AddPostOptionsLine("Before 0123456789012345678901234567890123456789 After");

            string help = local.ToString();

            string[] lines = help.Split(new string[] { Environment.NewLine }, StringSplitOptions.None);
            Assert.AreEqual("Before ", lines[1]);
            Assert.AreEqual("0123456789012345678901234567890123456789", lines[2]);
            Assert.AreEqual("012 After", lines[3]);
            Assert.AreEqual("Before ", lines[lines.Length - 3]);
            Assert.AreEqual("0123456789012345678901234567890123456789", lines[lines.Length - 2]);
            Assert.AreEqual(" After", lines[lines.Length - 1]);
        }
    }
}
#endif
