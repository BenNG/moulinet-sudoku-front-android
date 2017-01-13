package moulinet.tech.moulinet_sudoku_app;

import android.content.res.AssetManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("opencv_java3");

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        String initialStateOfThePuzzle = grabNumbers("s0", getAssets());
        Log.i("LOL", initialStateOfThePuzzle);



        String solvedPuzzle = sudokuSolver("006400750005082060007306089050130900093000840002048070580209600070860200029003400");


        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(solvedPuzzle);

        String[] filenames = getFilenameInAssets(getAssets());

        System.out.println(filenames);
        Log.i("LOL", filenames[0]);

        String content = getFileContent(getAssets());
        Log.i("LOL", content);

        Log.i("LOL", solvedPuzzle);



    }


    private native String[] getFilenameInAssets(AssetManager pAssetManager);
    private native String getFileContent(AssetManager pAssetManager);
    private native String sudokuSolver(String initialStateOfTheSudoku);

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String grabNumbers(String fileName, AssetManager pAssetManager);

}
