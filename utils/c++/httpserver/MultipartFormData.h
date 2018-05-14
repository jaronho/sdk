/**********************************************************************
* Author:	jaron.ho
* Date:		2018-04-11
* Brief:	multipart form-data parser
**********************************************************************/
#ifndef _MULTIPART_FORM_DATA_H_
#define	_MULTIPART_FORM_DATA_H_

#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include "HttpServer.h"

class HttpField;

/* multipart form-data */
class MultipartFormData {
public:
    MultipartFormData(void);
    ~MultipartFormData(void);

    bool parse(const std::string& contentType, const char* data, size_t length, std::map<std::string, HttpField*>* fields);

private:
    void processData(void);
    bool findStartingBoundaryAndTruncData(void);
    bool waitForHeadersEndAndParseThem(void);
    bool processContentOfTheField(void);
    size_t boundaryPositionInDataCollector(void);
    void truncateDataCollectorFromTheBeginning(size_t n);
    bool parseHeaders(const std::string& headers);

private:
    static unsigned int const STATUS_LOOKING_FOR_STARTING_BOUNDARY = 1;
    static unsigned int const STATUS_PROCESSING_HEADERS = 2;
    static unsigned int const STATUS_PPROCESSING_FIELD_CONTENT = 3;

private:
    std::string mBoundary;
    char* mData;
    size_t mDataSize;
    std::map<std::string, HttpField*>* mFields;
    unsigned int mCurrentProcessStatus;
    std::string mCurrentProcessingFieldName;
};

#endif  /* _MULTIPART_FORM_DATA_H_ */
