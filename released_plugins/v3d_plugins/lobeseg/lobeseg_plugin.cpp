/* lobeseg_plugin.cpp
 * This plugin sperate the two optic lobes (OLs) and the center brain (CB) of fluit fly brain. Or seperate just one lobe and the center brain with suitable parameters.
 * June 20, 2011 : by Hanchuan Peng and Hang Xiao
 * June 28, 2011 : by Hanchuan Peng: add the info of the paper, etc. Put in the v3d_released plugins folder
 * April 18, 2012: by Jianlong Zhou, update the dofunc() interface
 */

#include "v3d_message.h"

#include "lobeseg_plugin.h"
#include "lobeseg_func.h"

Q_EXPORT_PLUGIN2(lobeseg, LobesegPlugin);

QStringList LobesegPlugin::menulist() const
{
	return QStringList()
		<<tr("two sides segmentation")
		<<tr("one-side-only segmentation")
	    <<tr("open the lobeseg paper download webpage")
		<<tr("about");
}

QStringList LobesegPlugin::funclist() const
{
	return QStringList()
		<<tr("lobeseg")
          <<tr("help");
}

void LobesegPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("two sides segmentation"))
	{
		lobeseg_two_sides(callback,parent);
	}
	else if (menu_name == tr("one-side-only segmentation"))
	{
		lobeseg_one_side_only(callback,parent);
	}
	else if (menu_name == tr("open the lobeseg paper download webpage"))
	{
		bool b_openurl_worked=QDesktopServices::openUrl(QUrl("http://penglab.janelia.org/papersall/docpdf/2010_Methods_flybrainlobeseg.pdf"));
		if (! b_openurl_worked)
			QMessageBox::warning(parent,
								 "Error opening download page", // title
								 "Please browse to\n"
								 "http://penglab.janelia.org/papersall/docpdf/2010_Methods_flybrainlobeseg.pdf\n"
								 "to download the PDF of this paper for this plugin");
	}
	else
	{
		v3d_msg(tr("This plugin segments the two optic lobes (OLs) and the center brain (CB) of  "
				   "3D confocal images of fluit fly brains. It can also segment just one lobe and"
				   "the center brain with suitable parameters.                                   "
				   "                                                                             "
				   "The method is published in Lam, S. ... and Peng, H. (2010) Segmentation of   "
				   "center brains and optic lobes in 3D confocal images of adult fruit fly brains"
				   "Methods, Vol.50, No.2, pp.63-69, 2010.                                       "
				   "                                                                             "
			       "This plugin is developed by Hanchuan Peng and Hang Xiao, June, 2011.         "));
	}
}

bool LobesegPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("lobeseg"))
	{
		return lobeseg(input, output);
	}
     else if (func_name == tr("help"))
	{
          printHelp();
          return true;
     }
}

