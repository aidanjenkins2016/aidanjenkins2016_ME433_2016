/* Copyright 2011-2013 Google Inc.
 * Copyright 2013 mike wakerly <opensource@hoho.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 * Project home page: https://github.com/mik3y/usb-serial-for-android
 */

package com.hoho.android.usbserial.examples;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ScrollView;
import android.widget.TextView;

import com.google.android.gms.appindexing.Action;
import com.google.android.gms.appindexing.AppIndex;
import com.google.android.gms.common.api.GoogleApiClient;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.util.HexDump;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

import java.io.IOException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

//from camera
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;

import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.WindowManager;
import android.widget.SeekBar;

import java.io.IOException;
import static android.graphics.Color.blue;
import static android.graphics.Color.green;
import static android.graphics.Color.red;

/**
 * Monitors a single {@link UsbSerialPort} instance, showing all data
 * received.
 *
 * @author mike wakerly (opensource@hoho.com)
 */

public class SerialConsoleActivity extends Activity implements TextureView.SurfaceTextureListener {
//public class SerialConsoleActivity extends Activity  {

    //camera lives inside of this class
    private Camera mCamera;
    private TextureView mTextureView;
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private SeekBar myControlG;
    private SeekBar myControlR;
    private TextView myTextViewR;
    private TextView myTextView;
    private TextView testTextview;

    //change resolution here!
    private Bitmap bmp = Bitmap.createBitmap(400, 300, Bitmap.Config.ARGB_8888);
    /**
     * ATTENTION: This was auto-generated to implement the App Indexing API.
     * See https://g.co/AppIndexing/AndroidStudio for more information.
     */
    private GoogleApiClient client;

    public SerialConsoleActivity() {
        super();
    }

    private Canvas canvas = new Canvas(bmp);
    private Paint paint1 = new Paint();
    private TextView mTextView;


    static long prevtime = 0; // for FPS calculation
    static int greenthresh = 0; //threshold value for green
    static int redthresh = 0;
   // static int send_val;

    //public static int COM = 300;
    //public static String sendString = String.valueOf(COM) + '\n';

    protected void onCreate(Bundle savedInstanceState) {
        //super.onCreate(savedInstanceState);
        //setContentView(R.layout.serial_console);

        //from original serialconsoleactivity oncreate
        super.onCreate(savedInstanceState);
        setContentView(R.layout.serial_console);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        mTitleTextView = (TextView) findViewById(R.id.demoTitle);
        mDumpTextView = (TextView) findViewById(R.id.consoleText);
        mScrollView = (ScrollView) findViewById(R.id.demoScroller);
        chkDTR = (CheckBox) findViewById(R.id.checkBoxDTR);
        chkRTS = (CheckBox) findViewById(R.id.checkBoxRTS);

        chkDTR.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                try {
                    sPort.setDTR(isChecked);
                } catch (IOException x) {
                }
            }
        });

        chkRTS.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                try {
                    sPort.setRTS(isChecked);
                } catch (IOException x) {
                }
            }
        });

        //from camera
        myControlG = (SeekBar) findViewById(R.id.seek1);
        myControlR = (SeekBar) findViewById(R.id.seek2);
        myTextView = (TextView) findViewById(R.id.textView01);
        myTextViewR = (TextView) findViewById(R.id.textView02);
        myTextView.setText("Green Level: ");
        myTextViewR.setText("Red Level: ");
        setMyControlGListener();
        setMyControlRListener();

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON); // keeps the screen from turning off

        mSurfaceView = (SurfaceView) findViewById(R.id.surfaceview);
        mSurfaceHolder = mSurfaceView.getHolder();

        mTextureView = (TextureView) findViewById(R.id.textureview);
        mTextureView.setSurfaceTextureListener(this);

        mTextView = (TextView) findViewById(R.id.cameraStatus);

        paint1.setColor(0xffff0000); // red
        paint1.setTextSize(24);


        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        client = new GoogleApiClient.Builder(this).addApi(AppIndex.API).build();
    }



    private void setMyControlRListener() {

        int redstart_threshold = 35; //ENTER STARTING THRESHOLD HERE AFTER TESTING WITH THE GREEN/BLACK OVERLAY
        myControlR.setMax(255); //max value for slider
        myControlR.setProgress(redstart_threshold);
        myTextViewR.setText("Red Level: " + redstart_threshold);

        myControlR.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {


            int progressChanged = 0;
            Camera.Size bestSize = null;
            //List<Camera.Size> sizeList=mCamera.getParameters().getSupportedPreviewSizes();


            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

                progressChanged = progress;
                myTextViewR.setText("Red Level: " + progress);
                redthresh = progress;
            }

            public int setThreshold() {
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

    private void setMyControlGListener() {

        int greenstart_threshold = 19; //ENTER STARTING THRESHOLD HERE AFTER TESTING WITH THE GREEN/BLACK OVERLAY
        myControlG.setMax(255); //max value for slider
        myControlG.setProgress(greenstart_threshold);
        myTextView.setText("Green Level: " + greenstart_threshold);

        myControlG.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {


            int progressChanged = 0;
            Camera.Size bestSize = null;
            //List<Camera.Size> sizeList=mCamera.getParameters().getSupportedPreviewSizes();


            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

                progressChanged = progress;
                myTextView.setText("Green Level: " + progress);
                greenthresh = progress;
            }

            public int setThreshold() {
                return progressChanged;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }

            //snippet to send COM value essentially

        });






    }


    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mCamera = Camera.open();
        Camera.Parameters parameters = mCamera.getParameters();
        parameters.setPreviewSize(640, 480); //resolution here!!
        parameters.setColorEffect(Camera.Parameters.EFFECT_NONE); // black and white
        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_INFINITY); // no autofocusing
        parameters.setFlashMode(Camera.Parameters.FLASH_MODE_TORCH); // flash always on
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

    /// the important function
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        // Invoked every time there's a new Camera preview frame
        mTextureView.getBitmap(bmp);

        final Canvas c = mSurfaceHolder.lockCanvas();
        if (c != null) {

            int[] pixels = new int[bmp.getWidth()];
            int startY = 150; // which row in the bitmap to analyse to read
            // only look at one row in the image
            bmp.getPixels(pixels, 0, bmp.getWidth(), 0, startY, bmp.getWidth(), 1); // (array name, offset inside array, stride (size of row), start x, start y, num pixels to read per row, num rows to read)

            // pixels[] is the RGBA data (in black an white).
            // instead of doing center of mass on it, decide if each pixel is dark enough to consider black or white
            // then do a center of mass on the thresholded array
            int[] thresholdedPixels = new int[bmp.getWidth()];
            int wbTotal = 0; // total mass
            int wbCOM = 0; // total (mass time position)


            //RUN THIS FOR LOOP FOR CALIBRATION
            int[] thresholdedColors = new int[bmp.getWidth()];
            for (int j = 0; j < bmp.getHeight(); j++) {
                for (int i = 0; i < bmp.getWidth(); i++) {
                    // sum the red, green and blue, subtract from 255 to get the darkness of the pixel.
                    // if it is greater than some value (600 here), consider it black
                    // play with the 600 value if you are having issues reliably seeing the line
                    if ((red(pixels[i])) + 10 < redthresh && (green(pixels[i])) > greenthresh) {
                        thresholdedPixels[i] = 0; //
                        thresholdedColors[i] = Color.rgb(0, 200, 0);// show as green
                    } else if ((red(pixels[i])) > redthresh && (green(pixels[i])) > greenthresh) {
                        thresholdedPixels[i] = 255 * 3;
                        thresholdedColors[i] = Color.rgb(100, 50, 0); // show as brown
                    } else {
                        thresholdedColors[i] = Color.rgb(0, 0, 255);
                        thresholdedPixels[i] = 255 * 3;//black
                    }

                    if (j == startY) {
                        wbTotal = wbTotal + thresholdedPixels[i];
                        wbCOM = wbCOM + thresholdedPixels[i] * i;
                    }

                }
                bmp.setPixels(thresholdedColors, 0, bmp.getWidth(), 0, j, bmp.getWidth(), 1);
            }

            /*int[] thresholdedColors = new int[bmp.getWidth()];
            for (int j = 0; j < bmp.getHeight(); j++){
                for (int i = 0; i < bmp.getWidth(); i++) {
                    // sum the red, green and blue, subtract from 255 to get the darkness of the pixel.
                    // if it is greater than some value (600 here), consider it black
                    // play with the 600 value if you are having issues reliably seeing the line
                    if (255 - (red(pixels[i])) > greenthresh) {
                        thresholdedPixels[i] = 255 * 3; //white
                        thresholdedColors[i] = Color.rgb(255, 255, 0);//make the high red and green areas black
                    } else {
                        thresholdedPixels[i] = 0;//black
                        thresholdedColors[i] = Color.rgb(0, 255, 0); // make the not high red and green areas green
                    }

                    if(j==startY) {
                        wbTotal = wbTotal + thresholdedPixels[i];
                        wbCOM = wbCOM + thresholdedPixels[i] * i;
                    }

                }
                bmp.setPixels(thresholdedColors, 0, bmp.getWidth(), 0, j, bmp.getWidth(), 1);
            }*/
            //RUN THIS FOR LOOP FOR THE COMPETITION
                /*or (int i = 0; i < bmp.getWidth(); i++) {
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

                }*/


            int COM;

            //watch out for divide by 0
            if (wbTotal <= 0) {
                COM = bmp.getWidth() / 2;
            } else {
                COM = wbCOM / wbTotal;
            }

            int i=COM;
            String sendString1 = String.valueOf(i) + '\n';
            try {
                sPort.write(sendString1.getBytes(), 10); // 10 is the timeout
            } catch (IOException e) {}

            // draw a circle where you think the COM is
            canvas.drawCircle(COM, startY, 5, paint1);

            // also write the value as text
            canvas.drawText("COM = " + COM, 10, 200, paint1);
            c.drawBitmap(bmp, 0, 0, null);
            mSurfaceHolder.unlockCanvasAndPost(c);

            // calculate the FPS to see how fast the code is running
            long nowtime = System.currentTimeMillis();
            long diff = nowtime - prevtime;
            mTextView.setText("FPS " + 1000 / diff);
            prevtime = nowtime;

        }
    }

///////////////////////////////////////////////////////////

    //stuf from exsting serialconsoleactivity
    private final String TAG = SerialConsoleActivity.class.getSimpleName();

    /**
     * Driver instance, passed in statically via
     * {@link #show(Context, UsbSerialPort)}.
     * <p/>
     * <p/>
     * This is a devious hack; it'd be cleaner to re-create the driver using
     * arguments passed in with the {@link #startActivity(Intent)} intent. We
     * can get away with it because both activities will run in the same
     * process, and this is a simple demo.
     */
    private static UsbSerialPort sPort = null;

    //this stuff can go
    private TextView mTitleTextView;
    private TextView mDumpTextView;
    private ScrollView mScrollView;
    private CheckBox chkDTR;
    private CheckBox chkRTS;

    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();

    private SerialInputOutputManager mSerialIoManager;

    private final SerialInputOutputManager.Listener mListener =
            new SerialInputOutputManager.Listener() {

                @Override
                public void onRunError(Exception e) {
                    Log.d(TAG, "Runner stopped.");
                }

                @Override
                public void onNewData(final byte[] data) {
                    SerialConsoleActivity.this.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            SerialConsoleActivity.this.updateReceivedData(data);
                        }
                    });
                }
            };

     /*@Override
   public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.serial_console);
        mTitleTextView = (TextView) findViewById(R.id.demoTitle);
        mDumpTextView = (TextView) findViewById(R.id.consoleText);
        mScrollView = (ScrollView) findViewById(R.id.demoScroller);
        chkDTR = (CheckBox) findViewById(R.id.checkBoxDTR);
        chkRTS = (CheckBox) findViewById(R.id.checkBoxRTS);

        chkDTR.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                try {
                    sPort.setDTR(isChecked);
                }catch (IOException x){}
            }
        });

        chkRTS.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                try {
                    sPort.setRTS(isChecked);
                }catch (IOException x){}
            }
        });

    }*/


    @Override
    protected void onPause() {
        super.onPause();
        stopIoManager();
        if (sPort != null) {
            try {
                sPort.close();
            } catch (IOException e) {
                // Ignore.
            }
            sPort = null;
        }
        finish();
    }

    void showStatus(TextView theTextView, String theLabel, boolean theValue) {
        String msg = theLabel + ": " + (theValue ? "enabled" : "disabled") + "\n";
        theTextView.append(msg);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "Resumed, port=" + sPort);
        if (sPort == null) {
            mTitleTextView.setText("No serial device.");
        } else {
            final UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);

            UsbDeviceConnection connection = usbManager.openDevice(sPort.getDriver().getDevice());
            if (connection == null) {
                mTitleTextView.setText("Opening device failed");
                return;
            }

            try {
                sPort.open(connection);
                sPort.setParameters(115200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);

                showStatus(mDumpTextView, "CD  - Carrier Detect", sPort.getCD());
                showStatus(mDumpTextView, "CTS - Clear To Send", sPort.getCTS());
                showStatus(mDumpTextView, "DSR - Data Set Ready", sPort.getDSR());
                showStatus(mDumpTextView, "DTR - Data Terminal Ready", sPort.getDTR());
                showStatus(mDumpTextView, "DSR - Data Set Ready", sPort.getDSR());
                showStatus(mDumpTextView, "RI  - Ring Indicator", sPort.getRI());
                showStatus(mDumpTextView, "RTS - Request To Send", sPort.getRTS());

                //snippet to send COM value essentially
                int i = 110;
                String sendString = String.valueOf(i) + '\n';
                try {
                    sPort.write(sendString.getBytes(), 10); // 10 is the timeout
                } catch (IOException e) {
                }

            } catch (IOException e) {
                Log.e(TAG, "Error setting up device: " + e.getMessage(), e);
                mTitleTextView.setText("Error opening device: " + e.getMessage());
                try {
                    sPort.close();
                } catch (IOException e2) {
                    // Ignore.
                }
                sPort = null;
                return;
            }
            mTitleTextView.setText("Serial device: " + sPort.getClass().getSimpleName());
        }
        onDeviceStateChange();
    }

    private void stopIoManager() {
        if (mSerialIoManager != null) {
            Log.i(TAG, "Stopping io manager ..");
            mSerialIoManager.stop();
            mSerialIoManager = null;
        }
    }

    private void startIoManager() {
        if (sPort != null) {
            Log.i(TAG, "Starting io manager ..");
            mSerialIoManager = new SerialInputOutputManager(sPort, mListener);
            mExecutor.submit(mSerialIoManager);
        }
    }

    private void onDeviceStateChange() {
        stopIoManager();
        startIoManager();
    }

    private void updateReceivedData(byte[] data) {
        final String message = "Read " + data.length + " bytes: \n"
                + HexDump.dumpHexString(data) + "\n\n";
        mDumpTextView.append(message);
        mScrollView.smoothScrollTo(0, mDumpTextView.getBottom());
        byte[] sData = {'a', 0};
        try {
            sPort.write(sData, 10);
        } catch (IOException e) {
        }
    }

    /**
     * Starts the activity, using the supplied driver instance.
     *
     * @param context //@param driver
     */
    static void show(Context context, UsbSerialPort port) {
        sPort = port;
        final Intent intent = new Intent(context, SerialConsoleActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_NO_HISTORY);
        context.startActivity(intent);
    }

    @Override
    public void onStart() {
        super.onStart();

        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        client.connect();
        Action viewAction = Action.newAction(
                Action.TYPE_VIEW, // TODO: choose an action type.
                "SerialConsole Page", // TODO: Define a title for the content shown.
                // TODO: If you have web page content that matches this app activity's content,
                // make sure this auto-generated web page URL is correct.
                // Otherwise, set the URL to null.
                Uri.parse("http://host/path"),
                // TODO: Make sure this auto-generated app URL is correct.
                Uri.parse("android-app://com.hoho.android.usbserial.examples/http/host/path")
        );
        AppIndex.AppIndexApi.start(client, viewAction);
    }

    @Override
    public void onStop() {
        super.onStop();

        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        Action viewAction = Action.newAction(
                Action.TYPE_VIEW, // TODO: choose an action type.
                "SerialConsole Page", // TODO: Define a title for the content shown.
                // TODO: If you have web page content that matches this app activity's content,
                // make sure this auto-generated web page URL is correct.
                // Otherwise, set the URL to null.
                Uri.parse("http://host/path"),
                // TODO: Make sure this auto-generated app URL is correct.
                Uri.parse("android-app://com.hoho.android.usbserial.examples/http/host/path")
        );
        AppIndex.AppIndexApi.end(client, viewAction);
        client.disconnect();
    }
//change serial_console.xml to the camera layout
}
