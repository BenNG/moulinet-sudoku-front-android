package moulinet.tech.moulinet_sudoku_app;

        import java.io.FileOutputStream;
        import java.util.List;

        import org.opencv.android.JavaCameraView;

        import android.content.Context;
        import android.hardware.Camera;
        import android.hardware.Camera.PictureCallback;
        import android.hardware.Camera.Size;
        import android.util.AttributeSet;
        import android.util.Log;

public class Tutorial3View extends JavaCameraView {

    private static final String TAG = "Sample::Tutorial3View";
    private String mPictureFileName;

    public Tutorial3View(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public List<Size> getResolutionList() {
        return mCamera.getParameters().getSupportedPreviewSizes();
    }

    public void setResolution(Size resolution) {
        disconnectCamera();
        mMaxHeight = resolution.height;
        mMaxWidth = resolution.width;
        connectCamera(getWidth(), getHeight());
    }

    public Size getResolution() {
        return mCamera.getParameters().getPreviewSize();
    }

}