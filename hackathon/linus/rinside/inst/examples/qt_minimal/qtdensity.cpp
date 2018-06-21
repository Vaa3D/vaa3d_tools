// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
// Qt usage example for RInside, inspired by the standard 'density
// sliders' example for other GUI toolkits -- this time with SVG
//
// Copyright (C) 2011 - 2013  Dirk Eddelbuettel and Romain Francois

#include "qtdensity.h"

QtDensity::QtDensity(RInside & R) : m_R(R)
{
    m_bw = 100;                 // initial bandwidth, will be scaled by 100 so 1.0
    m_kernel = 0;               // initial kernel: gaussian
    m_cmd = "c(rnorm(100,0,1), rnorm(50,5,1))"; // simple mixture
    m_R["bw"] = m_bw;           // pass bandwidth to R, and have R compute a temp.file name
    m_tempfile = QString::fromStdString(Rcpp::as<std::string>(m_R.parseEval("tfile <- tempfile()")));
    m_svgfile = QString::fromStdString(Rcpp::as<std::string>(m_R.parseEval("sfile <- tempfile()")));
    setupDisplay();
}

void QtDensity::setupDisplay(void)  {
    QWidget *window = new QWidget;
    window->setWindowTitle("Qt and RInside demo: density estimation");

    m_svg = new QSvgWidget();
    runRandomDataCmd();         // also calls plot()

    QHBoxLayout *lowerlayout = new QHBoxLayout;
    lowerlayout->addWidget(m_svg);

    QVBoxLayout *outer = new QVBoxLayout;
    outer->addLayout(lowerlayout);
    window->setLayout(outer);
    window->show();
}

void QtDensity::plot(void) {
    const char *kernelstrings[] = { "gaussian", "epanechnikov", "rectangular", "triangular", "cosine" };
    m_R["bw"] = m_bw;
    m_R["kernel"] = kernelstrings[m_kernel]; // that passes the string to R
    std::string cmdin = "library(ggplot2);";
    m_R.parseEvalQ(cmdin);
    std::string cmd0 = "svg(width=8,height=6,pointsize=10,filename=tfile); ";
    //std::string cmd1 = "plot(density(y, bw=bw/100, kernel=kernel),col='green', xlim=range(y)+c(-2,2), main=\"Kernel: ";
    std::string cmd1_2 = "df=data.frame(y=y);";
    std::string cmd1_3 = "p <- ggplot(df, aes(x=y)) + geom_density(color='green') + geom_point(aes(x=y, y=rep(0, length(y)), colour = rgb(1,0,0), alpha = 0.25)) + annotate(geom = 'text', label = 'vaa3d.org', x = Inf, y = Inf, hjust = 1.2, vjust = 2, colour='lightblue');";
    std::string cmd1_4 = "print(p); dev.off()";
    //std::string cmd2 = "\"); points(y, rep(0, length(y)), pch=16, col=rgb(0,0,0,1/4));  dev.off()";
    //std::string cmd = cmd0 + cmd1 + kernelstrings[m_kernel] + cmd2; // stick the selected kernel in the middle
    std::string cmd = cmd0 + cmd1_2 + cmd1_3 + cmd1_4; // stick the selected kernel in the middle
    m_R.parseEvalQ(cmd);
    filterFile();           	// we need to simplify the svg file for display by Qt 
    m_svg->load(m_svgfile);
}

void QtDensity::getBandwidth(int bw) {
    if (bw != m_bw) {
        m_bw = bw;
        plot();
    }
}

void QtDensity::getKernel(int kernel) {
    if (kernel != m_kernel) {
        m_kernel = kernel;
        plot();
    }
}

void QtDensity::getRandomDataCmd(QString txt) {
    m_cmd = txt;
}

void QtDensity::runRandomDataCmd(void) {
    std::string cmd = "y2 <- " + m_cmd.toStdString() + "; y <- y2";
    m_R.parseEvalQNT(cmd);
    plot();                     // after each random draw, update plot with estimate
}

void QtDensity::filterFile() {
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
