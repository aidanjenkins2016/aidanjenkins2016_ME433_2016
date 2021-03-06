package com.example.aidan.aidanscameraapp;

// libraries
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.TextView;
import java.io.IOException;
import static android.graphics.Color.blue;
import static android.graphics.Color.green;
import static android.graphics.Color.red;

public class MainActivity extends Activity implements TextureView.SurfaceTextureListener {
    //SeekBar myControl; //initializations for slider
    //TextView myTextView;
    private Camera mCamera;
    private TextureView mTextureView;
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private SeekBar myControl;
    private TextView myTextView;

    //change resolution here!
    private Bitmap bmp = Bitmap.createBitmap(400,300,Bitmap.Config.ARGB_8888);


    public MainActivity() {
        super();
    }

    private Canvas canvas = new Canvas(bmp);
    private Paint paint1 = new Paint();
    private TextView mTextView;

    static long prevtime = 0; // for FPS calculation
    static int greenthresh=0 ; //threshold value for green

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        myControl = (SeekBar) findViewById(R.id.seek1);
        myTextView = (TextView) findViewById(R.id.textView01);
        myTextView.setText("Green Level!");
        setMyControlListener();

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON); // keeps the screen from turning off

        mSurfaceView = (SurfaceView) findViewById(R.id.surfaceview);
        mSurfaceHolder = mSurfaceView.getHolder();

        mTextureView = (TextureView) findViewById(R.id.textureview);
        mTextureView.setSurfaceTextureListener(this);

        mTextView = (TextView) findViewById(R.id.cameraStatus);

        paint1.setColor(0xffff0000); // red
        paint1.setTextSize(24);
    }
   private void setMyControlListener() {

       int start_threshold=330; //ENTER STARTING THRESHOLD HERE AFTER TESTING WITH THE GREEN/BLACK OVERLAY
       myControl.setMax(510); //max value for slider
       myControl.setProgress(start_threshold);
       myTextView.setText("Threshold: "+start_threshold);

       myControl.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {


            int progressChanged = 0;
            Camera.Size bestSize=null;
            //List<Camera.Size> sizeList=mCamera.getParameters().getSupportedPreviewSizes();


            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

                progressChanged = progress;
                myTextView.setText("Threshold: "+progress);
                greenthresh=progress;
            }

            public int setThreshold(){
                return progressChanged;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }



        });
    }

    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mCamera = Camera.open();
        Camera.Parameters parameters = mCamera.getParameters();
        parameters.setPreviewSize(640, 480); //resolution here!!
        parameters.setColorEffect(Camera.Parameters.EFFECT_MONO); // black and white
        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_INFINITY); // no autofocusing
        //parameters.setFlashMode(Camera.Parameters.FLASH_MODE_TORCH); // flash always on
        mCamera.setParameters(parameters);
        mCamera.setDisplayOrientation(90); // rotate to portrait mode

        try {
            mCamera.setPreviewTexture(surface);
            mCamera.startPreview();
        } catch (IOException ioe) {
            // Something bad happened
        }
    }

    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        // Ignored, Camera does all the work for us
    }

    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        mCamera.stopPreview();
        mCamera.release();
        return true;
    }

    // the important function
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        // Invoked every time there's a new Camera preview frame
        mTextureView.getBitmap(bmp);

        final Canvas c = mSurfaceHolder.lockCanvas();
        if (c != null) {

            int[] pixels = new int[bmp.getWidth()];
            int startY = 50; // which row in the bitmap to analyse to read
            // only look at one row in the image
            bmp.getPixels(pixels, 0, bmp.getWidth(), 0, startY, bmp.getWidth(), 1); // (array name, offset inside array, stride (size of row), start x, start y, num pixels to read per row, num rows to read)

            // pixels[] is the RGBA data (in black an white).
            // instead of doing center of mass on it, decide if each pixel is dark enough to consider black or white
            // then do a center of mass on the thresholded array
            int[] thresholdedPixels = new int[bmp.getWidth()];
            int wbTotal = 0; // total mass
            int wbCOM = 0; // total (mass time position)

            /*
            //RUN THIS FOR LOOP FOR CALIBRATION
            int[] thresholdedColors = new int[bmp.getWidth()];
            for (int j = 0; j < bmp.getHeight(); j++){
                for (int i = 0; i < bmp.getWidth(); i++) {
                    // sum the red, green and blue, subtract from 255 to get the darkness of the pixel.
                    // if it is greater than some value (600 here), consider it black
                    // play with the 600 value if you are having issues reliably seeing the line
                    if (255 * 2 - (red(pixels[i]) + green(pixels[i]) ) > greenthresh) {
                        thresholdedPixels[i] = 255*3; //white
                        thresholdedColors[i] = Color.rgb(0, 0, 0);//make the high red and green areas black
                    } else {
                        thresholdedPixels[i] = 0;//black
                        thresholdedColors[i] = Color.rgb(0, 255, 0); // make the not high red and green areas green
                    }

                    wbTotal = wbTotal + thresholdedPixels[i];
                    wbCOM = wbCOM + thresholdedPixels[i] * i;

                }
                bmp.setPixels(thresholdedColors, 0, bmp.getWidth(), 0, j, bmp.getWidth(), 1);
            }
            */
                //RUN THIS FOR LOOP FOR THE COMPETITION
                for (int i = 0; i < bmp.getWidth(); i++) {
                    // sum the red, green and blue, subtract from 255 to get the darkness of the pixel.
                    // if it is greater than some value (600 here), consider it black
                    // play with the 600 value if you are having issues reliably seeing the line
                    if (255 * 2 - (red(pixels[i]) + green(pixels[i])) > greenthresh) {
                        thresholdedPixels[i] = 255 * 3; //white
                        //thresholdedColors[i] = Color.rgb(0, 0, 0);//make the high red and green areas black
                    } else {
                        thresholdedPixels[i] = 0;//black
                        //thresholdedColors[i] = Color.rgb(0, 255, 0); // make the not high red and green areas green
                    }

                    wbTotal = wbTotal + thresholdedPixels[i];
                    wbCOM = wbCOM + thresholdedPixels[i] * i;

                }



            int COM;
            //watch out for divide by 0
            if (wbTotal<=0) {
                COM = bmp.getWidth()/2;
            }
            else {
                COM = wbCOM/wbTotal;
            }


            // draw a circle where you think the COM is
            canvas.drawCircle(COM, startY, 5, paint1);

            // also write the value as text
            canvas.drawText("COM = " + COM, 10, 200, paint1);
            c.drawBitmap(bmp, 0, 0, null);
            mSurfaceHolder.unlockCanvasAndPost(c);

            // calculate the FPS to see how fast the code is running
            long nowtime = System.currentTimeMillis();
            long diff = nowtime - prevtime;
            mTextView.setText("FPS " + 1000/diff);
            prevtime = nowtime;
        }
    }
}