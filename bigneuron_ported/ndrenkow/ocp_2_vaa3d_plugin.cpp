/* ocp_2_vaa3d_plugin.cpp
 * Plugin to import OCP data to Vaa3D
 * 2015-6-1 : by JHU-APL
 */
 
#include "v3d_message.h"
#include <vector>
#include "ocp_2_vaa3d_plugin.h"
#include <streambuf>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <math.h>
#include <exception>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
using namespace std;
#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif

using namespace boost::asio;
using boost::asio::ip::tcp;

Q_EXPORT_PLUGIN2(ocp_2_vaa3d, OCP2Vaa3D);
 
QStringList OCP2Vaa3D::menulist() const
{
	return QStringList() 
		<<tr("OCP_to_Vaa3D")
		<<tr("about");
}

QStringList OCP2Vaa3D::funclist() const
{
	return QStringList()
		<<tr("Import_data")
		<<tr("help");
}

string SERVER = "openconnecto.me";

//---------------------------------------------------------------------------------------------------------
void GetFile(const std::string& serverName, const std::string& getCommand, std::ofstream& outFile){
    boost::asio::io_service io_service;

    // Get a list of endpoints corresponding to the server name.
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(serverName, "http");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    // Try each endpoint until we successfully establish a connection.
    tcp::socket socket(io_service);
    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endpoint_iterator != end)
    {
        socket.close();
        socket.connect(*endpoint_iterator++, error);
    }

    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    request_stream << "GET " << getCommand << " HTTP/1.0\r\n";
    request_stream << "Host: " << serverName << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    // Send the request.
    boost::asio::write(socket, request);

    // Read the response status line.
    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n");

    // Check that response is OK.
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);


    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(socket, response, "\r\n\r\n");

    // Process the response headers.
    std::string header;
    while (std::getline(response_stream, header) && header != "\r")
    {
    }

    // Write whatever content we already have to output.
    if (response.size() > 0)
    {
        outFile << &response;
    }
    // Read until EOF, writing data to output as we go.
    while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
    {
        outFile << &response;
    }
}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
std::vector<string> get_public_tokens(std::string server_name){
    vector<string> tokens;

    boost::filesystem::path tempfile = boost::filesystem::unique_path();
    boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();
    boost::filesystem::temp_directory_path();
    temp_path /= tempfile;
    const std::string tempstr = temp_path.native();  // optional

    // Get tokens from server
    string getCommand = "/ocp/ca/public_tokens/";
    std::ofstream outFile(tempstr.c_str(), std::ofstream::out | std::ofstream::binary);

    GetFile(server_name, getCommand, outFile);
    outFile.close();

    // Read in token data
    std::ifstream t(tempstr.c_str());
    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)),
               std::istreambuf_iterator<char>());

    str = str.substr(1, str.size() - 2);
    boost::trim(str);

    // Clost stream and delete file
    t.close();
    if(boost::filesystem::exists(tempstr))
        boost::filesystem::remove(tempstr);


    // Split into vector of strings
    vector<string> SplitVec; // #2: Search for tokens
    boost::split( SplitVec, str, boost::is_any_of("\"") );

    for(std::vector<std::string>::iterator it = SplitVec.begin(); it != SplitVec.end(); ++it) {
        if (it->empty()) {
            continue;
        }
        if (it->find(',') == std::string::npos)
        {
            tokens.push_back(*it);
        }
    }

    return tokens;
}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
boost::filesystem::path get_proj_info_file(std::string server_name, std::string token){
    boost::filesystem::path tempfile = boost::filesystem::unique_path();
    boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();
    boost::filesystem::temp_directory_path();
    temp_path /= tempfile;
    temp_path += ".h5";
    const std::string tempstr = temp_path.native();  // optional

    // Get tokens from server
    string getCommand = "/ocp/ca/" + token + "/projinfo/";
    std::ofstream outFile(tempstr.c_str(), std::ofstream::out | std::ofstream::binary);

    GetFile(server_name, getCommand, outFile);
    outFile.close();

    return temp_path;
}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
std::vector<string> get_image_set_dimensions(boost::filesystem::path h5_file){
    std::vector<string> dim_vect;
    vector<int> slice_range;


    const H5std_string FILE_NAME( h5_file.native() );
    const H5std_string DATASET_GROUP( "DATASET" );
    try
        {
            /*
             * Turn off the auto-printing when failure occurs so that we can
             * handle the errors appropriately
             */
            //Exception::dontPrint();

            // Open the file
            H5File file( FILE_NAME, H5F_ACC_RDONLY );
            Group top_group = file.openGroup(DATASET_GROUP);

            // Grab the slice range
            DataSet slice_data = top_group.openDataSet("SLICERANGE");

            // get the dataspace
            DataSpace dspace_slice = slice_data.getSpace();

            // get the size of the dataset
            hsize_t rank_slice;
            hsize_t dims_slice[2];
            rank_slice = dspace_slice.getSimpleExtentDims(dims_slice, NULL); // rank = 1

            // Define the memory dataspace
            hsize_t dimsm_slice[1];
            dimsm_slice[0] = dims_slice[0];
            DataSpace memspace_slice (1,dimsm_slice);

            // Assuming INT - create a vector the same size as the dataset
            slice_range.resize(dims_slice[0]);

            slice_data.read(slice_range.data(), PredType::NATIVE_INT, memspace_slice, dspace_slice);



            // grab the DATASET/IMAGE_SIZE Group
            Group image_size_group = top_group.openGroup("IMAGE_SIZE");

            // Iterate through the list of resolutions
            hsize_t num_res = image_size_group.getNumObjs();
            std::string resolution = "";
            for (int ii = 0; ii < num_res; ii++){
                // Get the Dataset Name
                resolution = image_size_group.getObjnameByIdx(ii);
                DataSet res_data = image_size_group.openDataSet(resolution);

                // get the dataspace
                DataSpace dspace = res_data.getSpace();

                // get the size of the dataset
                hsize_t rank;
                hsize_t dims[2];
                rank = dspace.getSimpleExtentDims(dims, NULL); // rank = 1

                // Define the memory dataspace
                hsize_t dimsm[1];
                dimsm[0] = dims[0];
                DataSpace memspace (1,dimsm);

                // Assuming INT - create a vector the same size as the dataset
                vector<int> data;
                data.resize(dims[0]);

                res_data.read(data.data(), PredType::NATIVE_INT, memspace, dspace);

                // Save string
                stringstream ss;
                ss << resolution;
                ss << ",";
                ss << data[0];
                ss << ",";
                ss << data[1];
                ss << ",";
                ss << slice_range[0];
                ss << ",";
                ss << slice_range[1];

                dim_vect.push_back(ss.str());

                std::cout<<ss.str()<<std::endl;

            }


            file.close();

        }  // end of try block
        // catch failure caused by the H5File operations
        catch( FileIException error )
        {
            error.printError();
            return dim_vect;
        }
        // catch failure caused by the DataSet operations
        catch( DataSetIException error )
        {
            error.printError();
            return dim_vect;
        }
        // catch failure caused by the DataSpace operations
        catch( DataSpaceIException error )
        {
            error.printError();
            return dim_vect;
        }
        // catch failure caused by the DataSpace operations
        catch( DataTypeIException error )
        {
            error.printError();
            return dim_vect;
        }

    return dim_vect;
}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
std::vector<string> get_channels(boost::filesystem::path h5_file){
    std::vector<string> channel_vect;

    /*
     * Turn off the auto-printing when failure occurs so that we can
     * handle the errors appropriately
     */
    //Exception::dontPrint();

    // Open the file
    H5File file( h5_file.native(), H5F_ACC_RDONLY );
    if (H5Lexists(file.getId(), "CHANNELS", H5P_DEFAULT) != true) {
        return channel_vect;
    }
    Group channel_group = file.openGroup("CHANNELS");


    // Iterate through the list of resolutions
    hsize_t num_ch = channel_group.getNumObjs();
    std::string channel_str = "";
    for (int ii = 0; ii < num_ch; ii++){
        // Get the Dataset Name
        channel_str = channel_group.getObjnameByIdx(ii);

        channel_vect.push_back(channel_str);

    }

    file.close();

    return channel_vect;
}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
void delete_hdf5_file(boost::filesystem::path h5_file){
    if(boost::filesystem::exists(h5_file.native()))
        boost::filesystem::remove(h5_file.native());
}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
unsigned char* get_image_data(const std::string &server, const std::string &url, hsize_t* dims){
    // Create Temp Dir
    boost::filesystem::path tempfile = boost::filesystem::unique_path();
    boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();
    boost::filesystem::temp_directory_path();
    temp_path /= tempfile;
    temp_path += ".h5";
    const std::string tempstr = temp_path.native();  // optional
    printf("%s\n",tempstr.c_str());

    // Get image data from server
    std::ofstream outFile(tempstr.c_str(), std::ofstream::out | std::ofstream::binary);
    GetFile(server, url, outFile);
    outFile.close();

    // Open the file
    H5File file( tempstr, H5F_ACC_RDONLY );

    // Grab the dataset
    DataSet cutout = file.openDataSet("CUTOUT");

    // get the dataspace
    DataSpace dspace = cutout.getSpace();

    // get the size of the dataset
    hsize_t rank;
    rank = dspace.getSimpleExtentDims(dims, NULL);

    // Define the memory dataspace
    //hsize_t dimsm[1];
    //dimsm[0] = dims[0];
    DataSpace memspace(rank,dims);

    // Assuming INT - create a vector the same size as the dataset
    // TODO: MAKE LOOK AT DATATYPE
    int data_size = dims[0] * dims[1] * dims[2];
    unsigned char* data_out = new unsigned char[data_size];

    cutout.read(data_out, PredType::NATIVE_UCHAR, memspace, dspace);

    file.close();

    // Delete hdf5 file
    if(boost::filesystem::exists(tempstr))
        boost::filesystem::remove(tempstr);

    return data_out;
}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
unsigned char* get_channel_data(const std::string &server, const std::string &url, const std::vector<std::string> &channels, hsize_t* dims)
{
    // Create Temp Dir
    boost::filesystem::path tempfile = boost::filesystem::unique_path();
    boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();
    boost::filesystem::temp_directory_path();
    temp_path /= tempfile;
    temp_path += ".h5";
    const std::string tempstr = temp_path.native();  // optional

    // Get image data from server
    std::ofstream outFile(tempstr.c_str(), std::ofstream::out | std::ofstream::binary);
    GetFile(server, url, outFile);
    outFile.close();

    // Open the file
    H5File file( tempstr, H5F_ACC_RDONLY );

    // Loop over all channels
    unsigned char* data_out;
    int num_channels = channels.size();
    dims[3] = num_channels;
    // Grab the dataset
    Group top_group = file.openGroup("CUTOUT");
    for (int iCh=0; iCh < num_channels; ++iCh){
        // get the specific channel cutout
        DataSet cutout = top_group.openDataSet(channels[iCh]);

        // get the dataspace
        DataSpace dspace = cutout.getSpace();

        // get the size of the dataset
        hsize_t rank;
        rank = dspace.getSimpleExtentDims(dims, NULL);

        // Define the memory dataspace
        DataSpace memspace(rank,dims);

        // Assuming INT - create a vector the same size as the dataset
        // TODO: MAKE LOOK AT DATATYPE
        int data_size = dims[0] * dims[1] * dims[2] * 2; //16-bit
        unsigned char* channel_data = new unsigned char[data_size]; // 16-bit
        cutout.read(channel_data, PredType::NATIVE_UINT16, memspace, dspace);

        if (iCh == 0) {
            data_out = new unsigned char[num_channels * data_size];
        }

        int offset = iCh * data_size; // 16-bit
        for (int idx=0; idx < data_size; ++idx) {
            data_out[idx + offset] = channel_data[idx];
        }
    }

    file.close();

    printf("%s\n",tempstr.c_str());
    // Delete hdf5 file
    if(boost::filesystem::exists(tempstr))
        boost::filesystem::remove(tempstr);

    return data_out;

}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
void select_project_token(const vector<string> &ocp_projects, string &token)
{
    // -------------------
    //  Select project
    // -------------------
    QDialog dialog;
    QFormLayout form(&dialog);
    QComboBox projects(&dialog);

    for (int i=0; i<ocp_projects.size(); ++i) {
        projects.addItem(QString(ocp_projects[i].c_str()));
    }
    form.addRow(&projects);

    QLabel *label = new QLabel;
    label->setText(ocp_projects[0].c_str());

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    QObject::connect(&projects, SIGNAL(currentIndexChanged(QString)), label, SLOT(setText(QString)));

    if (dialog.exec() == QDialog::Accepted) {
        token = label->text().toStdString();
        qDebug() << QString(token.c_str());
    } else {
        token = "";
    }
}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
int check_extents(const string &resolution, const string &x_ex, const string &y_ex, const string &z_ex) {

    // Extract volume dimensions from comma-separated strings
    int counter = 0;
    string token = resolution;
    vector<int> extents; //x_end,y_end,z_start,z_end
    bool exist_commas = true;
    while (exist_commas) {
        size_t found = token.find_first_of(",");
        if (found != std::string::npos) {
            if (counter > 0) {
                // First token will be resolution level
                extents.push_back(atoi(token.substr(0,found).c_str()));
            }
            token = token.substr(found+1);
        } else if (!token.empty()) {
            // Last value in string
            extents.push_back(atoi(token.c_str()));
            exist_commas = false;
        } else {
            exist_commas = false;
        }
        ++counter;
    }

    // Extract extents from comma-separated strings
    int xstart,xstop,ystart,ystop,zstart,zstop;
    size_t found = x_ex.find_first_of(",");
    if (found != std::string::npos) {
        xstart = atoi(x_ex.substr(0,found).c_str());
        xstop = atoi(x_ex.substr(found+1).c_str());
    } else {
        return 0;
    }

    found = y_ex.find_first_of(",");
    if (found != std::string::npos) {
        ystart = atoi(y_ex.substr(0,found).c_str());
        ystop = atoi(y_ex.substr(found+1).c_str());
    } else {
        return 0;
    }

    found = z_ex.find_first_of(",");
    if (found != std::string::npos) {
        zstart = atoi(z_ex.substr(0,found).c_str());
        zstop = atoi(z_ex.substr(found+1).c_str());
    } else {
        return 0;
    }

    // Check extents
    printf("%d,%d,%d,%d\n",extents[0],extents[1],extents[2],extents[3]);
    printf("%d,%d,%d,%d,%d,%d\n",xstart,xstop,ystart,ystop,zstart,zstop);
    return (xstop > xstart) && (xstart >= 0) && (xstop <= extents[0]) &&
           (ystop > ystart) && (ystart >= 0) && (ystop <= extents[1]) &&
           (zstop > zstart) && (zstart >= extents[2]) && (zstop <= extents[3]);

}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
void retrieve_ocp_url(const string &project, const vector<string> &channels,
                      const vector<string> &dimensions, string &data_url, vector<string> &selected_channels)
{
    int num_resolutions = dimensions.size();
    int num_channels = channels.size();

    // -------------------
    //  Select dataset parameters
    // -------------------
    QDialog dialog;
    QGridLayout *mainLayout = new QGridLayout;

    // Image Resolution Selection
    QGroupBox *resolutionBox = new QGroupBox(QString("Resolution (level,x_end,y_end,z_start,z_end):"));
    QVBoxLayout *resolutionBoxLayout = new QVBoxLayout();
    QRadioButton *buttons[num_resolutions];
    for (int i=0; i<num_resolutions; ++i) {
        buttons[i] = new QRadioButton(dimensions[i].c_str());
        resolutionBoxLayout->addWidget(buttons[i]);
        if (i == 0) {
            buttons[i]->setChecked(true);
        }
    }
    resolutionBox->setLayout(resolutionBoxLayout);
    mainLayout->addWidget(resolutionBox,0,0,2,1);

    // Channel Selection
    QGroupBox *channelBox = new QGroupBox(QString("Channel:"));
    QVBoxLayout *channelBoxLayout = new QVBoxLayout;
    QCheckBox *channelButtons[num_channels];
    for (int i=0; i<num_channels; ++i) {
        channelButtons[i] = new QCheckBox(channels[i].c_str());
        channelBoxLayout->addWidget(channelButtons[i]);
    }
    channelBox->setLayout(channelBoxLayout);
    mainLayout->addWidget(channelBox,0,1,2,1);

    // User volume info
    QGroupBox *formBox = new QGroupBox("OCP Data Parameters:");
    QFormLayout *formLayout = new QFormLayout;

    // Add some text above the fields
    QList<QLineEdit *> fields;
    QLineEdit *xLine = new QLineEdit;
    QString xLabel = QString("X [start,stop]:");
    formLayout->addRow(xLabel, xLine);
    fields << xLine;

    QLineEdit *yLine = new QLineEdit;
    QString yLabel = QString("Y [start,stop]:");
    formLayout->addRow(yLabel, yLine);
    fields << yLine;

    QLineEdit *zLine = new QLineEdit;
    QString zLabel = QString("Z [start,stop]:");
    formLayout->addRow(zLabel, zLine);
    fields << zLine;

    formBox->setLayout(formLayout);
    mainLayout->addWidget(formBox,0,2,1,1);

    // Ok/cancel button
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel);

    mainLayout->addWidget(buttonBox,2,1,1,1);
    QObject::connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Prepare dialog for viewing
    dialog.setLayout(mainLayout);
    string x_extents,y_extents,z_extents;
    string all_selected_channels = "";
    string resolution = "";
    string full_extents = "";
    if (dialog.exec() == QDialog::Accepted) {
        // Retrieve resolution value
        for (int i=0; i<num_resolutions; ++i) {
            if (buttons[i]->isChecked()) {
                std::size_t found = dimensions[i].find_first_of(",");
                resolution = dimensions[i].substr(0,found);
                full_extents = dimensions[i];
                break;
            }
        }
        printf("Selected resolution: %s\n",resolution.c_str());

        // Retrieve channel value
        int num_checked = 0;
        for (int i=0; i<num_channels; ++i) {
            if (channelButtons[i]->isChecked()) {
                ++num_checked;
                if (num_checked > 3) {
                    QMessageBox *msg_box = new QMessageBox;
                    msg_box->setText("Number of selected channels exceeds 3");
                    msg_box->setInformativeText("Only the first 3 selected channels from the list will be displayed.");
                    msg_box->setStandardButtons(QMessageBox::Ok);
                    msg_box->setDefaultButton(QMessageBox::Ok);
                    int ret = msg_box->exec();
                    break;
                }
                selected_channels.push_back(channels[i]);
                all_selected_channels += "," + channels[i];
            }
        }
        if (!all_selected_channels.empty()) {
            // remove leading ","
            all_selected_channels = all_selected_channels.substr(1);
        }

        // Retrieve data extents
        x_extents = xLine->text().toStdString();
        y_extents = yLine->text().toStdString();
        z_extents = zLine->text().toStdString();

        int valid_extents = check_extents(full_extents,x_extents,y_extents,z_extents);
        if (!valid_extents) {
            QMessageBox *msg_box = new QMessageBox;
            msg_box->setText("Extents are invalid. Exiting ocp_2_vaa3d plugin...");
            msg_box->setStandardButtons(QMessageBox::Ok);
            msg_box->setDefaultButton(QMessageBox::Ok);
            int ret = msg_box->exec();
            return;
        }

        // Debugging
        qDebug() << QString(project.c_str());
        qDebug() << QString(resolution.c_str());
        qDebug() << QString(all_selected_channels.c_str());
        qDebug() << QString(x_extents.c_str());
        qDebug() << QString(y_extents.c_str());
        qDebug() << QString(z_extents.c_str());
    } else {
        qDebug() << "Exiting";
        data_url = "";
        return;
    }

    // -------------------
    //  Format URL string
    // -------------------
    string base_url = "/ocp/ca";
    char buffer[200];
    if (num_channels == 0) {
        // Omit channel from the URL
        sprintf(buffer,"%s/%s/hdf5/%s/%s/%s/%s/",base_url.c_str(),project.c_str(),resolution.c_str(),x_extents.c_str(),y_extents.c_str(),z_extents.c_str());
    } else {
        sprintf(buffer,"%s/%s/hdf5/%s/%s/%s/%s/%s/",base_url.c_str(),project.c_str(),all_selected_channels.c_str(),resolution.c_str(),x_extents.c_str(),y_extents.c_str(),z_extents.c_str());
    }
    data_url = buffer;

    qDebug() << QString(data_url.c_str());
}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------

unsigned char* permute_hdf5(const unsigned char *old_data, const hsize_t* hdf5_dims, int pixel_depth)
{
    /*
        WARNING: This code will not work in the current state.  It is not used at the moment.
    */
    printf("Permuting image dims\n");

    // Pixel depth = num bytes per pixel
    hsize_t data_size = hdf5_dims[0] * hdf5_dims[1] * hdf5_dims[2] * pixel_depth;
    unsigned char* new_data = new unsigned char[data_size];

    hsize_t depth = hdf5_dims[0];
    hsize_t height = hdf5_dims[1];
    hsize_t width = hdf5_dims[2];
    printf("Data dims (D,H,W): %d,%d,%d\n",depth,height,width);
    printf("Data size: %d\n",data_size);

    for (hsize_t d=0; d < depth; ++d) {
        for (hsize_t h=0; h < height; ++h) {
            for (hsize_t w=0; w < width; ++w) {
                printf("%d,%d,%d\n",w,h,d);
                hsize_t ind = w + (hsize_t)h*width + (hsize_t)d*width*height;

                hsize_t h5x,h5y,h5z;
                hsize_t hdf5_idx = ind; //(h5x + (hsize_t)fmax(h5y-1,0)*width + (hsize_t)fmax(h5z-1,0)*height*width) * pixel_depth;

                hsize_t v3x,v3y,v3z;
                hsize_t vaa3d_idx = (w + (hsize_t)h*width + (hsize_t)d*width*height) * pixel_depth;//(v3x + (hsize_t)fmax(v3y-1,0)*height + (hsize_t)fmax(v3z-1,0)*height*width) * pixel_depth;

//                printf("%d\n",ind);
//                printf("%d,%d,%d,%d\n",h5x,h5y,h5z,hdf5_idx);
//                printf("%d,%d,%d,%d\n",v3x,v3y,v3z,vaa3d_idx);
                for (hsize_t i_byte=0; i_byte < pixel_depth; ++i_byte) {
                    new_data[vaa3d_idx + i_byte] = old_data[hdf5_idx + i_byte];
                } //byte
            }
        }
    }


//    for (hsize_t x=0; x < hdf5_dims[2]; ++x) {
//        for (hsize_t z=0; z < hdf5_dims[0]; ++z) {
//            // column-major
//            for (hsize_t y=0; y < hdf5_dims[1]; ++y) {
//                hsize_t old_idx = (y + x*hdf5_dims[1] + z*hdf5_dims[0]) * pixel_depth;

//                hsize_t new_z = floor(old_idx/(hdf5_dims[2]*hdf5_dims[1]));
//                hsize_t new_y = floor((old_idx-(new_z*hdf5_dims[1]*hdf5_dims[2]))/hdf5_dims[1]);
//                hsize_t new_x = old_idx % hdf5_dims[2];

//                // Row major in vaa3d
//                hsize_t new_idx = (new_x + fmax(new_y-1,0)*hdf5_dims[2] + fmax(new_z-1,0)*hdf5_dims[2]*hdf5_dims[1]) * pixel_depth;//(x + y*hdf5_dims[1] + z*hdf5_dims[0]) * pixel_depth;
//                printf("%d,%d\n",old_idx,new_idx);
//                for (hsize_t i_byte=0; i_byte < pixel_depth; ++i_byte) {
//                    new_data[new_idx + i_byte] = old_data[old_idx + i_byte];
//                } //byte
//            } //y
//        } //z
//    } //x


    return new_data;
}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------

void OCP2Vaa3D::import_from_ocp(V3DPluginCallback2 &callback, QWidget *parent)
{

    // -------------------
    // Retrieve list of projects from OCP url
    // -------------------
    vector<string> ocp_tokens;
    qDebug() << QString("Get public tokens");
    ocp_tokens = get_public_tokens(SERVER);

    // -------------------
    //  Get project data
    // -------------------
    string token;
    select_project_token(ocp_tokens,token);
    boost::filesystem::path proj_info_file;
    proj_info_file = get_proj_info_file(SERVER, token);

    // -------------------
    //  Display options and retrieve URL
    // -------------------
    string ocp_url;
    vector<string> channels,dimensions;
    dimensions = get_image_set_dimensions(proj_info_file);
    channels = get_channels(proj_info_file);
    vector<string> selected_channels;
    retrieve_ocp_url(token,channels,dimensions,ocp_url,selected_channels);
    delete_hdf5_file(proj_info_file);
    if (ocp_url.empty()) {
        qDebug() << QString("No URL retrieved");
        return;
    }

    // -------------------
    //  Retrieve data
    // -------------------
    unsigned char* data;
    Image4DSimple p4DImage;
    if (selected_channels.empty()) {
        hsize_t data_dims[3];
        data = get_image_data(SERVER,ocp_url,data_dims);
        // HDF5 represents data as Z,Y,X so note the swap in dimensions
        p4DImage.setData(data, data_dims[2], data_dims[1], data_dims[0], 1, V3D_UINT8);
    } else {
        hsize_t data_dims[4];
        data = get_channel_data(SERVER,ocp_url,selected_channels,data_dims);
        printf("Data dims (X,Y,Z,C): %d,%d,%d,%d/n",data_dims[2],data_dims[1],data_dims[0],data_dims[3]);
        // HDF5 represents data as Z,Y,X so note the swap in dimensions
        p4DImage.setData(data, data_dims[2], data_dims[1], data_dims[0], data_dims[3], V3D_UINT16);
    }
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, &p4DImage);
    callback.updateImageWindow(newwin);
}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
void OCP2Vaa3D::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("OCP_to_Vaa3D"))
	{
        import_from_ocp(callback,parent);
	}
	else
	{
		v3d_msg(tr("Plugin to import OCP data to Vaa3D. "
			"Developed by JHU-APL, 2015-6-1"));
	}
}

bool OCP2Vaa3D::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("Import_data"))
	{
        v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}


