package moulinet.tech.moulinet_sudoku_app;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;
import android.widget.TextView;

import org.opencv.android.Utils;
import org.opencv.core.Mat;

public class MainActivity extends AppCompatActivity {
// Log.i("LOL", initialStateOfThePuzzle);


    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("opencv_java3");

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        String fileNumber = "s0";

        // load image
        Mat img_input = new Mat();
        loadImage(fileNumber, img_input.getNativeObjAddr(), getAssets());

        solve(img_input.getNativeObjAddr(), getAssets());

        // setting up display stuff
        ImageView imageVIewInput = (ImageView)findViewById(R.id.imageViewInput);
        Bitmap bitmapInput = Bitmap.createBitmap(img_input.cols(), img_input.rows(), Bitmap.Config.ARGB_8888);
        Utils.matToBitmap(img_input, bitmapInput);
        imageVIewInput.setImageBitmap(bitmapInput);

    }

    private native void loadImage(String fileName, long img, AssetManager pAssetManager);
    private native void solve(long img, AssetManager pAssetManager);

}
