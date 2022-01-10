#include "QPrefixSuffixLineEdit.h"

using namespace terastitcher;

// Fortunately, this function gets called every single time the line edit is typed into. We use it to intercept and negate any editing of the prefix or suffix.
QPrefixSuffixValidator::State QPrefixSuffixValidator::validate ( QString & input, int & pos ) const {

	//qDebug() << "calling validate: incoming: '" << input << "'.";

	// possible situations:
	/*	- normal editing inside the allowed "actual" text area
		- just backspaced over the prefix separator
		- just deleted the suffix separator
		- clicked over into the prefix or suffix regions and typed some text that shouldn't be there. (We want to capture this character and put it back where it belongs)
		- setText editing that gave a correct string, but with the pos at the very end.
		   */


	// Did they just delete the prefix separator? (ie: by hitting backspace) If so, erase everything prior to their position, and add it back
    if(!input.contains(terastitcher::separator1())) {
		input.remove(0, pos);
        input.prepend(terastitcher::separator1());
		input.prepend(prefix_);
		pos = prefix_.length()+1;
		//qDebug() << "  validate: missing sep1:" << input;
	}

	// So we still have the separator.  What's before it might be bad or good.
	else {
        int indexOfSep = input.indexOf(terastitcher::separator1());

		// all good?
		if(input.left(indexOfSep) == prefix_) {
			// do nothing...
			//qDebug() << "  validate: prefix is good.";
		}

		else {

			// Did they add or replace a char in the prefix? In this case, the pos will be before the separator, and the number of chars before the separator will be the same or longer than it should be.
			if( pos > 0 && pos <= indexOfSep && indexOfSep >= prefix_.length() ){
				QChar rescued = input.at(pos - 1);
				input.remove(0, indexOfSep + 1);
				input.prepend(rescued);
				pos = prefix_.length() + 2;
				//qDebug() << "  validate: edited prefix:" << input;
			}
			// any other damage (or even non-damage, for now) we handle by deleting everything up to and including the separator, and adding it all back.
			else {
				input.remove(0, indexOfSep + 1);
				//qDebug() << "  validate: generic prefix fix:" << input;
			}
			// add the prefix back...
            input.prepend(terastitcher::separator1());
			input.prepend(prefix_);

			//qDebug() << "  validate: end of prefix fix:" << input;
		}
	}

	// Did they just delete the suffix sep?
    if(!input.contains(terastitcher::separator2())) {
		input.chop(input.length() - pos);
        input.append(terastitcher::separator2());
		input.append(suffix_);
		//qDebug() << "  validate: missing sep2:" << input;
	}
	// otherwise, we still have the separator
	else {
        int indexOfSep = input.lastIndexOf(terastitcher::separator2());

		// all good?
		if(input.mid(indexOfSep+1) == suffix_) {
			//qDebug() << "  validate: suffix is good";
			// do nothing
		}
		else {
			// did they add or delete a character in the suffix?
			if(pos > indexOfSep && input.length() - indexOfSep >= suffix_.length()) {
				QChar rescued = input.at(pos-1);
				input.chop(input.length() - indexOfSep);
				input.append(rescued);
				//qDebug() << "  validate: edited suffix:" << input;
			}
			// otherwise... (any other kind of damage, handle generically)
			else {
				input.chop(input.length() - indexOfSep);
				//qDebug() << "  validate: generic suffix fix:" << input;
			}

            input.append(terastitcher::separator2());
			input.append(suffix_);
			//qDebug() << "  validate: end of suffix fix:" << input;
		}
	}


	// make sure they keep their position within where it's allowed to be...
	if(pos < prefix_.length() + 1 )
		pos = prefix_.length() + 1;

	if( pos > input.length() - suffix_.length() - 1)
		pos = input.length() - suffix_.length() - 1;

	//qDebug() << "calling validate: output: '" << input << "'.";
	return QValidator::Acceptable;
}

QPrefixSuffixLineEdit::QPrefixSuffixLineEdit(const QString& prefix, const QString& suffix, QWidget *parent) :
		QLineEdit(parent)
{
	setPrefix(prefix);
	setSuffix(suffix);
	setValidator(&validator_);
}


void QPrefixSuffixLineEdit::setPrefix(const QString& prefix)
{
	// clear the old prefix
	QString originalText = QLineEdit::text();
    if(originalText.contains(terastitcher::separator1()))
        originalText.remove(0, originalText.indexOf(terastitcher::separator1()) + 1);

	// ad the new one
    originalText.prepend(terastitcher::separator1());
	originalText.prepend(prefix_ = prefix);
	QLineEdit::setText(originalText);

	validator_.setPrefix(prefix_);

}
void QPrefixSuffixLineEdit::setSuffix(const QString& suffix)
{
	QString oldText = QLineEdit::text();

	// clear the old suffix
    if(oldText.contains(terastitcher::separator2()))
        oldText.chop(oldText.length() - oldText.lastIndexOf(terastitcher::separator2()));

    oldText.append(terastitcher::separator2());
	oldText.append(suffix_ = suffix);
	QLineEdit::setText(oldText);

	validator_.setSuffix(suffix_);
}

