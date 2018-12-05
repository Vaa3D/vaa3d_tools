// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4;  tab-width: 8; -*-
//
// Qt usage example for RInside, inspired by the standard 'density
// sliders' example for other GUI toolkits
//
// Copyright (C) 2011  Dirk Eddelbuettel and Romain Francois


#include <QApplication>
#include "Rinstancing.h"

bool instantiateR(int argc, char *argv[], vector<double> radius, vector<double> crossings)
{

    RInside R(argc, argv);  		// create an embedded R instance

    qDebug()<<"Rinstance created";

    R.assign(radius,"radius");
    R.assign(crossings,"crossings");

    QtSholl qtsholl(R);		// pass R inst. by reference
}

QtSholl::QtSholl(RInside & R) : m_R(R)
{
    m_tempfile = QString::fromStdString(Rcpp::as<std::string>(m_R.parseEval("tfile <- tempfile()")));
    m_svgfile = QString::fromStdString(Rcpp::as<std::string>(m_R.parseEval("sfile <- tempfile()")));
    setupDisplay();
}

void QtSholl::setupDisplay(void)  {
    QWidget *window = new QWidget;
    window->setWindowTitle("Sholl Analysis");
    window->resize(1600,1200);

    m_svg = new QSvgWidget();
    plot();

    QHBoxLayout *lowerlayout = new QHBoxLayout;
    lowerlayout->addWidget(m_svg);

    QVBoxLayout *outer = new QVBoxLayout;
    outer->addLayout(lowerlayout);
    window->setLayout(outer);
    window->show();
}

void QtSholl::plot(void) {

    std::string cmdin = "library(ggplot2);";
    m_R.parseEvalQ(cmdin);

    std::string cmd0 = "svg(width=8,height=6,pointsize=10,filename=tfile); ";

    std::string cmd1_1 = "df=data.frame(x=radius,y=crossings);";
    std::string cmd1_2 = "p <- ggplot(df, aes(x=x,y=y)) + geom_line(color='green') + annotate(geom = 'text', label = 'vaa3d.org', x = Inf, y = Inf, size=6, hjust = 1.2, vjust = 2, colour='lightblue');";
    std::string cmd1_3 = "p <- p + labs(title='Sholl Analysis', x='Distance from soma [pixels]', y='Number of intersections') + theme(plot.title = element_text(size=15)) + theme(axis.text = element_text(size=10)) + theme(axis.title = element_text(size=13));";
    std::string cmd1_4 = "ggsave(file='sholl.svg', plot=p, width=8, height=6);";
    std::string cmd1_5 = "print(p); dev.off()";


    std::string cmd = cmd0 + cmd1_1 + cmd1_2 + cmd1_3 + cmd1_4 + cmd1_5; // stick the selected kernel in the middle
    m_R.parseEvalQ(cmd);
    filterFile();           	// we need to simplify the svg file for display by Qt
    m_svg->load(m_svgfile);
}

void QtSholl::filterFile() {
    // cairoDevice creates richer SVG than Qt can display
    // but per Michaele Lawrence, a simple trick is to s/symbol/g/ which we do here
    QFile infile(m_tempfile);
    infile.open(QFile::ReadOnly);
    QFile outfile(m_svgfile);
    outfile.open(QFile::WriteOnly | QFile::Truncate);

    QTextStream in(&infile);
    QTextStream out(&outfile);
    QRegExp rx1("<symbol");
    QRegExp rx2("</symbol");
    while (!in.atEnd()) {
        QString line = in.readLine();
        line.replace(rx1, "<g"); // so '<symbol' becomes '<g ...'
        line.replace(rx2, "</g");// and '</symbol becomes '</g'
        out << line << "\n";
    }
    infile.close();
    outfile.close();
}
