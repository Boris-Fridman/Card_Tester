#ifndef ____ProgImage_h__
#define ____ProgImage_h__

/*======================================================================================================================*/

#define PATH_FILE_NAME_LEN 500

/*======================================================================================================================*/

/**
 * @brief Initilizes image. Checks existance of the given file with image (".bin") file.
 * 
 * @code 
 * int InitImage(char ImgFilePathName[]);
 * @code
 * 
 * @param ImgFilePathName Name of the image file including path.
 * 
 * @return "0" if succeed or "-1" if failed.
 */
int InitImage(char ImgFilePathName[]);

/**
 * @brief Sends the image with update existed in ".bin" file to the network segment by segment. If the respond wasn't given the function will wait infinitly.
 * 
 * @code
 * int SendImageToNetwork();
 * @code
 * 
 * @return "0" in anycase.
 */
int SendImageToNetwork();

/**
 * @brief Closes the image file.
 * 
 * @code
 * void CloseImage();
 * @code
 */
void CloseImage();


/*======================================================================================================================*/

#endif  /*  ____ProgImage_h__*/

