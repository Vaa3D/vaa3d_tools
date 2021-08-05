#include "rpm.h"


void points_norm(Mat x, Mat& T, Mat& xn) {

	int length = x.rows;

	// compute the center of every points's XYZ coordinate
	Mat x_avg;
	reduce(x, x_avg, 0, CV_REDUCE_AVG, CV_64F);	

	// center the points
	Mat	xc;
	xc = x - Mat::ones(length, 1, CV_64F)*x_avg;

	//compute the average distance of every point to the origin
	Mat tmp;
	reduce(xc.mul(xc), tmp, 1, CV_REDUCE_SUM, CV_64F);	
	sqrt(tmp, tmp);
	reduce(tmp, tmp, 0, CV_REDUCE_AVG, CV_64F);	

	//compute the scale factor
	double s = 1 / tmp.at<double>(0, 0);
	xn = s * xc;

	//compute the transformation matrix
	T = (Mat_<double>(4, 4) <<
		s, 0, 0, -s * x_avg.at<double>(0, 0),
		0, s, 0, -s * x_avg.at<double>(0, 1),
		0, 0, s, -s * x_avg.at<double>(0, 2),
		0, 0, 0, 1);
	T = T.t();			//xn=x*T
}

void pca(Mat x, Mat y, Mat& Tpca, Mat& xp, Mat& tar_vec) {

	int x_length = x.rows;
	int y_length = y.rows;
	int dim = x.cols;
	Mat	 vx, vy, tmp;

	PCACompute(x, tmp, vx);
	PCACompute(y, tmp, vy);
	tar_vec = vy;
	Mat pcatrans, xt;
	pcatrans = vx.inv()*vy;
	xt = x * pcatrans;			
	xp = xt.clone();				

	float theta[12] = { 0, 0, 0, 0, 0, 180, 0, 180, 0, 0, 180, 180 };

	Mat  tmp1, dis, near;
	Mat t = Mat::eye(3, 3, CV_64F);

	for (int i = 0; i < 11; i = i + 3) {
		double sita_x = theta[i] / 180 * CV_PI;
		double sita_y = theta[i + 1] / 180 * CV_PI;
		double sita_z = theta[i + 2] / 180 * CV_PI;

		Mat Rx = (Mat_<double>(3, 3) << 1, 0, 0, 0, cos(sita_x), sin(sita_x), 0, -sin(sita_x), cos(sita_x));
		Mat Ry = (Mat_<double>(3, 3) << cos(sita_y), 0, -sin(sita_y), 0, 1, 0, sin(sita_y), 0, cos(sita_y));
		Mat Rz = (Mat_<double>(3, 3) << cos(sita_z), sin(sita_z), 0, -sin(sita_z), cos(sita_z), 0, 0, 0, 1);

		x = xt * Rz*Ry*Rx;
		tmp = Mat::zeros(y_length, x_length, CV_64F);
		for (int j = 0; j < dim; j++) {
			tmp1 = (y.col(j) * Mat::ones(1, x_length, CV_64F) - Mat::ones(y_length, 1, CV_64F) * x.col(j).t());
			tmp = tmp + tmp1.mul(tmp1);
		}

		cv::sort(tmp, near, CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);			
		tmp1 = near.colRange(Range(0, 10));
		reduce(tmp1, tmp1, 0, CV_REDUCE_SUM);
		reduce(tmp1, tmp1, 1, CV_REDUCE_SUM);


		if (i > 0) {
			if (tmp1.at<double>(0, 0) < dis.at<double>(0, 0)) {
				dis = tmp1.clone();
				xp = x.clone();
				t = Rz * Ry * Rx;
				//cout << "pca adjust result t is:" << t << endl;
			}
		}
		else {
			dis = tmp1.clone();		
		}
	}
	Tpca = pcatrans*t;
	//	cout << "final pca estimation is:" << Tpca << endl;
}


Mat affine3d(Mat x, Mat y) {
	Mat T1norm, T2norm, xn, yn;
	points_norm(x, T1norm, xn);
	points_norm(y, T2norm, yn);

	int length = x.rows;
	Mat A = Mat::zeros(3 * length, 13, CV_64F);
	Mat xn4, tmp;
	hconcat(xn, Mat::ones(length, 1, CV_64F), xn4);
	xn4.copyTo(A(Range(0, length), Range(0, 4)));
	xn4.copyTo(A(Range(length, 2 * length), Range(4, 8)));
	xn4.copyTo(A(Range(2 * length, 3 * length), Range(8, 12)));
	tmp = -yn;
	tmp.col(0).copyTo(A(Range(0, length), Range(12, 13)));
	tmp.col(1).copyTo(A(Range(length, 2 * length), Range(12, 13)));
	tmp.col(2).copyTo(A(Range(2 * length, 3 * length), Range(12, 13)));

	//A=u*¡°s¡±*v
	Mat u, s, v, h;
	SVD::compute(A, s, u, v);
	v(Range(12, 13), Range::all()).copyTo(h);
	h = h / h(Range(0, 1), Range(12, 13));
	h = h.t();

	Mat T = Mat::zeros(4, 4, CV_64F);
	T.at<double>(3, 3) = 1;
	tmp = h(Range(0, 4), Range::all()).t();
	tmp.copyTo(T(Range(0, 1), Range(0, 4)));
	tmp = h(Range(4, 8), Range::all()).t();
	tmp.copyTo(T(Range(1, 2), Range(0, 4)));
	tmp = h(Range(8, 12), Range::all()).t();
	tmp.copyTo(T(Range(2, 3), Range(0, 4)));
	invert(T2norm.t(), tmp);
	T = tmp*T*T1norm.t();
	return T;

}


Mat calc_vy(Mat vx, Mat y, double t) {
	int x_length = vx.rows;
	int y_length = y.rows;
	int dim = vx.cols;
	Mat	tmp = Mat::zeros(x_length, y_length, CV_64F);
	Mat tmp1;

	for (int i = 0; i < dim; i++) {
		tmp1 = (vx.col(i)*Mat::ones(1, y_length, CV_64F)- Mat::ones(x_length, 1, CV_64F)*y.col(i).t());
		tmp = tmp + tmp1.mul(tmp1);
	}

	Mat m, sy;

	exp(-tmp / t, tmp1);
	m = (1 / sqrt(t))*(tmp1);
	reduce(m, sy, 0, CV_REDUCE_SUM, CV_64F);
	tmp1 = Mat::ones(x_length, 1, CV_64F)*sy;
	m = m / tmp1;

	Mat vy;

	reduce(m, tmp, 1, CV_REDUCE_SUM, CV_64F);
	vy = m * y / (tmp * Mat::ones(1, dim, CV_64F));
	return vy;
}


Mat am_acol(Mat x, int flag) {
	Mat newx;
	int length = x.rows;
	if (flag) {
		newx = x(Range::all(), Range(0, 3));
	}
	else {
		hconcat(x, Mat::ones(length, 1, CV_64F), newx);
	}
	return newx;
}