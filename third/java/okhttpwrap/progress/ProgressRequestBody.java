package com.jaronho.sdk.third.okhttpwrap.progress;

import android.os.Message;

import com.jaronho.sdk.third.okhttpwrap.bean.ProgressMessage;
import com.jaronho.sdk.third.okhttpwrap.callback.ProgressCallback;
import com.jaronho.sdk.third.okhttpwrap.handler.OkMainHandler;

import java.io.IOException;

import okhttp3.MediaType;
import okhttp3.RequestBody;
import okio.Buffer;
import okio.BufferedSink;
import okio.ForwardingSink;
import okio.Okio;
import okio.Sink;

/**
 * 进度请求体
 */
public class ProgressRequestBody extends RequestBody {

    private final RequestBody originalRequestBody;
    private final ProgressCallback progressCallback;
    private BufferedSink bufferedSink;

    public ProgressRequestBody(RequestBody originalRequestBody, ProgressCallback progressCallback) {
        this.progressCallback = progressCallback;
        this.originalRequestBody = originalRequestBody;
    }

    @Override
    public MediaType contentType() {
        return originalRequestBody.contentType();
    }

    @Override
    public long contentLength() throws IOException {
        return originalRequestBody.contentLength();
    }

    @Override
    public void writeTo(BufferedSink originalSink) throws IOException {
        if (null == bufferedSink) {
            bufferedSink = Okio.buffer(sink(originalSink));
        }
        originalRequestBody.writeTo(bufferedSink);
        bufferedSink.flush();
    }

    private Sink sink(Sink originalSink) {
        return new ForwardingSink(originalSink) {
            long bytesWritten = 0L;
            long contentLength = 0L;
            @Override
            public void write(Buffer source, long byteCount) throws IOException {
                super.write(source, byteCount);
                if (contentLength == 0) {
                    contentLength = contentLength();
                }
                bytesWritten += byteCount;
                if (null != progressCallback) {
                    int percent = (int)((100 * bytesWritten) / contentLength);
                    progressCallback.onProgressAsync(percent, bytesWritten,contentLength,bytesWritten == contentLength);
                    // 主线程回调
                    Message msg = new ProgressMessage(OkMainHandler.PROGRESS_CALLBACK,
                            progressCallback,
                            percent,
                            bytesWritten,
                            contentLength,
                            bytesWritten == contentLength)
                            .build();
                    OkMainHandler.getInstance().sendMessage(msg);
                }
            }
        };
    }
}
