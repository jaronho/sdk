/**********************************************************************
* Author:	jaron.ho
* Date:		2018-04-11
* Brief:	multipart form-data parser
**********************************************************************/
#include "MultipartFormData.h"
//------------------------------------------------------------------------
MultipartFormData::MultipartFormData(void) {
    mFields = NULL;
    mData = NULL;
    mDataSize = 0;
    mCurrentProcessStatus = STATUS_LOOKING_FOR_STARTING_BOUNDARY;
}
//------------------------------------------------------------------------
MultipartFormData::~MultipartFormData(void) {
    if (mData) {
        delete mData;
    }
}
//------------------------------------------------------------------------
bool MultipartFormData::parse(const std::string& contentType, const char* data, size_t length, std::map<std::string, HttpField*>* fields) {
    if (!data || 0 == length || !fields) {
        return false;
    }
    std::string contentTypeTmp = contentType;
    std::transform(contentTypeTmp.begin(), contentTypeTmp.end(), contentTypeTmp.begin(), ::tolower);
    if (std::string::npos == contentTypeTmp.find("multipart/form-data;")) {
        return false;
    }
    size_t pos = contentTypeTmp.find("boundary=");
    if (std::string::npos == pos) {
        return false;
    }
    mBoundary = std::string("--") + contentType.substr(pos + 9, contentType.length() - pos);
    if (mBoundary.empty()) {
        return false;
    }
    if (!mData) {
        mData = new char[length];
    } else {
        mData = (char*)realloc(mData, mDataSize + length);
    }
    if (!mData) {
        return false;
    }
    memcpy(mData + mDataSize, data, length);
    mDataSize += length;
    mFields = fields;
    processData();
    return true;
}
//------------------------------------------------------------------------
void MultipartFormData::processData(void) {
    bool needToRepeat;
    do {
        needToRepeat = false;
        switch (mCurrentProcessStatus) {
        case STATUS_LOOKING_FOR_STARTING_BOUNDARY:
            if (findStartingBoundaryAndTruncData()) {
                mCurrentProcessStatus = STATUS_PROCESSING_HEADERS;
                needToRepeat = true;
            }
            break;
        case STATUS_PROCESSING_HEADERS:
            if (waitForHeadersEndAndParseThem()) {
                mCurrentProcessStatus = STATUS_PPROCESSING_FIELD_CONTENT;
                needToRepeat = true;
            }
            break;
        case STATUS_PPROCESSING_FIELD_CONTENT:
            if (processContentOfTheField()) {
                mCurrentProcessStatus = STATUS_LOOKING_FOR_STARTING_BOUNDARY;
                needToRepeat = true;
            }
            break;
        }
    } while (needToRepeat);
}

bool MultipartFormData::findStartingBoundaryAndTruncData(void) {
    size_t pos = boundaryPositionInDataCollector();
    if (pos >= 0) {
        truncateDataCollectorFromTheBeginning(pos + mBoundary.length());
        return true;
    }
    return false;
}

bool MultipartFormData::waitForHeadersEndAndParseThem(void) {
    for (size_t i = 0; i < mDataSize - 3; ++i) {
        if (13 == mData[i] && 10 == mData[i + 1] && 13 == mData[i + 2] && 10 == mData[i + 3]) {
            size_t headersLength = i;
            char* headers = new char[headersLength + 1];
            memset(headers, 0, headersLength + 1);
            memcpy(headers, mData, headersLength);
            if (!parseHeaders(headers)) {
                delete headers;
                return false;
            }
            truncateDataCollectorFromTheBeginning(i + 4);
            delete headers;
            return true;
        }
    }
    return false;
}

bool MultipartFormData::processContentOfTheField(void) {
    size_t pos = boundaryPositionInDataCollector();
    size_t contentLength = 0;
    if (pos >= 0) {
        /* 2 is the \r\n before boundary we do not need them */
        contentLength = pos - 2;
    } else {
        /* need to save +2 chars for \r\n chars before boundary */
        contentLength = mDataSize - (mBoundary.length() + 2);
    }
    if (contentLength > 0) {
        (*mFields)[mCurrentProcessingFieldName]->setContent(mData, contentLength);
        truncateDataCollectorFromTheBeginning(contentLength);
    }
    if (pos >= 0) {
        mCurrentProcessStatus = STATUS_LOOKING_FOR_STARTING_BOUNDARY;
        return true;
    }
    return false;
}

size_t MultipartFormData::boundaryPositionInDataCollector(void) {
    const char* boundary = mBoundary.c_str();
    size_t boundaryLength = mBoundary.length();
    if (mDataSize >= boundaryLength) {
        bool found = false;
        for (size_t i = 0; (i <= mDataSize - boundaryLength) && !found; ++i) {
            found = true;
            for (size_t j = 0; (j < boundaryLength) && found; ++j) {
                if (mData[i + j] != boundary[j]) {
                    found = false;
                }
            }
            if (found) {
                return i;
            }
        }
    }
    return -1;
}

void MultipartFormData::truncateDataCollectorFromTheBeginning(size_t n) {
    size_t truncatedDataCollectorLength = mDataSize - n;
    char* tmp = mData;
    mData = new char[truncatedDataCollectorLength];
    memcpy(mData, tmp + n, truncatedDataCollectorLength);
    mDataSize = truncatedDataCollectorLength;
    delete tmp;
}

bool MultipartFormData::parseHeaders(const std::string& headers) {
    /* check if it is form data */
    if (std::string::npos == headers.find("Content-Disposition: form-data;")) {
        return false;
    }
    /* find name */
    size_t namePos = headers.find("name=\"");
    if (std::string::npos == namePos) {
        return false;
    }
    size_t nameEndPos = headers.find("\"", namePos + 6);
    if (std::string::npos == nameEndPos) {
        return false;
    }
    /* generate multipart/form-data field */
    mCurrentProcessingFieldName = headers.substr(namePos + 6, nameEndPos - (namePos + 6));
    (*mFields)[mCurrentProcessingFieldName] = new HttpField();
    (*mFields)[mCurrentProcessingFieldName]->setName(mCurrentProcessingFieldName);
    /* find filename if exists */
    size_t filenamePos = headers.find("filename=\"");
    if (std::string::npos == filenamePos) {
        (*mFields)[mCurrentProcessingFieldName]->setType(HttpField::TYPE_TEXT);
    } else {
        (*mFields)[mCurrentProcessingFieldName]->setType(HttpField::TYPE_FILE);
        size_t filenameEndPos = headers.find("\"", filenamePos + 10);
        if (std::string::npos == filenameEndPos) {
            return false;
        }
        std::string filename = headers.substr(filenamePos + 10, filenameEndPos - (filenamePos + 10));
        (*mFields)[mCurrentProcessingFieldName]->setFilename(filename);
        /* find Content-Type if exists */
        size_t contentTypePos = headers.find("Content-Type: ");
        if (contentTypePos != std::string::npos) {
            size_t contentTypeEndPos = 0;
            for (size_t i = contentTypePos + 14; i < headers.length() && !contentTypeEndPos; ++i) {
                if (' ' == headers[i] || 10 == headers[i] || 13 == headers[i]) {
                    contentTypeEndPos = i - 1;
                }
            }
            std::string fileContentType = headers.substr(contentTypePos + 14, contentTypeEndPos - (contentTypePos + 14));
            (*mFields)[mCurrentProcessingFieldName]->setFileContentType(fileContentType);
        }
    }
    return true;
}
