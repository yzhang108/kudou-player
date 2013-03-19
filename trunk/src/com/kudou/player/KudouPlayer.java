package com.kudou.player;

import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.GestureDetector;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.GestureDetector.OnGestureListener;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.app.Activity;
import android.content.pm.ActivityInfo;

import org.libsdl.app.SDLActivity;
import org.libsdl.app.SDLSurface;

public class KudouPlayer extends Activity implements OnGestureListener {
	private String TAG = "TestPlayer";
	private SDLActivity mSoftPlayer = null;
	private View mHideContainer;
	private SeekBar mSeekBar;
	private TextView currentTime, totalTime;
	private View imgPlay;
	private String fileName = "/sdcard/test_1.mp4";
	private int totalDuration = 0;
	private Handler handler;
	private SeekUpdater seekUpdater = null;
	private GestureDetector gestureDetector = null;

	public final int MSG_LOAD_FINISHED = 10;
	public final int MSG_LOAD_UNFINISHED = 11;
	public final int MSG_OPEN_ERROR = 12;
	public final int MSG_OPEN_OK = 13;
	public final int MSG_SEEK_UPDATE = 30;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// ==================设置全屏=========================
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		// ===================设置亮度========================
		WindowManager.LayoutParams lp = getWindow().getAttributes();
		lp.screenBrightness = 0.5F;

		setContentView(R.layout.player);
		FrameLayout frameContainer = (FrameLayout) findViewById(R.id.framecontainer);

		findViews();
		gestureDetector = new GestureDetector(this);

		handler = new Handler() {
			public void handleMessage(Message msg) {
				if (!Thread.currentThread().isInterrupted()) {
					System.out.println("receive msg : " + msg.what);
					switch (msg.what) {
					case MSG_OPEN_OK:
						startPlayer();
						break;
					case MSG_OPEN_ERROR:
						break;
					case MSG_LOAD_FINISHED:
						break;
					case MSG_LOAD_UNFINISHED:
						break;
					case MSG_SEEK_UPDATE:
						Globals.ShowLog("MSG_SEEK_UPDATE");
						if (seekUpdater != null)
							seekUpdater.refresh();
						break;
					}
				}
				super.handleMessage(msg);
			}
		};

		Uri tmpUri = (Uri) this.getIntent().getData();
		if (tmpUri != null) {
			fileName = tmpUri.getPath();
		} else {

			Bundle bundle = this.getIntent().getExtras();
			if (bundle != null) {
				fileName = bundle.getString("PATH");
			}
		}
		mSoftPlayer = new SDLActivity(getApplication(), handler, fileName);

		SDLSurface surface = mSoftPlayer.getSDLSurface();

		FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
				LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT);
		params.gravity = Gravity.CENTER;
		surface.setLayoutParams(params);
		frameContainer.addView(surface);

	}

	public void startPlayer() {
		totalDuration = mSoftPlayer.getDuration();
		totalTime.setText(formatTime(totalDuration / 1000));
		if (seekUpdater == null) {
			seekUpdater = new SeekUpdater();
			seekUpdater.startIt();
		}
	}

	public void showLog(String s) {
		Log.i(TAG, s);
	}

	public String formatTime(long sec) {
		int h = (int) sec / 3600;
		int m = (int) (sec % 3600) / 60;
		int s = (int) sec % 60;
		if (h == 0)
			return String.format("%02d:%02d", m, s);
		else
			return String.format("%d:%02d:%02d", h, m, s);
	}

	public void findViews() {
		mSeekBar = (SeekBar) findViewById(R.id.progressbar);
		currentTime = (TextView) findViewById(R.id.currenttime);
		totalTime = (TextView) findViewById(R.id.totaltime);

		imgPlay = findViewById(R.id.img_vp_play);
		imgPlay.setOnClickListener(imgPlayListener);
		mSeekBar.setOnSeekBarChangeListener(mSeekBarChangeListener);
		mHideContainer = findViewById(R.id.hidecontainer);
		mHideContainer.setOnClickListener(mVisibleListener);
	}

	OnClickListener imgPlayListener = new OnClickListener() {
		@Override
		public void onClick(View v) {
			ImageView img = (ImageView) v;

			SDLActivity sp = mSoftPlayer;

			if (sp != null) {
				if (sp.isPlaying()) {
					img.setImageResource(R.drawable.vp_play);
					sp.stop();

				} else {
					img.setImageResource(R.drawable.vp_pause);
					sp.start();
				}
			}

		}
	};

	OnSeekBarChangeListener mSeekBarChangeListener = new OnSeekBarChangeListener() {
		@Override
		public void onStopTrackingTouch(SeekBar seekBar) {
			int totalTime, seekTo = 0;
			int progress = seekBar.getProgress();
			SDLActivity sp = mSoftPlayer;

			if (sp != null) {
				totalTime = sp.getDuration();
				seekTo = totalTime / 1000 * progress;
				sp.seekTo(seekTo);

			}

			System.out.println("Seeked to new progress: " + seekTo);
		}

		@Override
		public void onProgressChanged(SeekBar seekBar, int progress,
				boolean fromUser) {
			// TODO Auto-generated method stub

		}

		@Override
		public void onStartTrackingTouch(SeekBar seekBar) {
			// TODO Auto-generated method stub

		}
	};

	OnClickListener mVisibleListener = new OnClickListener() {
		public void onClick(View v) {
			Log.i("Test", "onClick mVisibleListener");
			if ((mHideContainer.getVisibility() == View.GONE)
					|| (mHideContainer.getVisibility() == View.INVISIBLE)) {
				if (seekUpdater != null) {
					seekUpdater.startIt();
					seekUpdater.refresh();
				}
				mHideContainer.setVisibility(View.VISIBLE);
			} else {
				mHideContainer.setVisibility(View.INVISIBLE);
				if (seekUpdater != null)
					seekUpdater.stopIt();
			}
		}
	};

	protected void onPause() {
		super.onPause();
		mSoftPlayer.exit();
		mSoftPlayer.onPause();
	}

	protected void onResume() {
		super.onResume();
		mSoftPlayer.onResume();

	}

	protected void onDestroy() {
		super.onDestroy();
		mSoftPlayer.onDestroy();
	}

	private class SeekUpdater {

		public void startIt() {
			handler.sendEmptyMessage(MSG_SEEK_UPDATE);
		}

		public void stopIt() {
			handler.removeMessages(MSG_SEEK_UPDATE);
		}

		public void refresh() {
			SDLActivity sp = mSoftPlayer;
			if (currentTime != null) {
				long playedDuration = 0;

				if (sp != null)
					playedDuration = sp.getCurrentPosition();

				currentTime.setText(formatTime(playedDuration / 1000));
				if (totalDuration != 0) {
					int progress = (int) ((1000 * playedDuration) / totalDuration);
					mSeekBar.setProgress(progress);
				}
			}
			handler.sendEmptyMessageDelayed(MSG_SEEK_UPDATE, 1000);

		}

	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		return gestureDetector.onTouchEvent(event);
	}

	@Override
	public boolean onDown(MotionEvent e) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
			float velocityY) {
		// TODO Auto-generated method stub
		Log.i("Test", "onFling");
		return false;
	}

	@Override
	public void onLongPress(MotionEvent e) {
		// TODO Auto-generated method stub

	}

	@Override
	public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
			float distanceY) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void onShowPress(MotionEvent e) {
		// TODO Auto-generated method stub

	}

	@Override
	public boolean onSingleTapUp(MotionEvent e) {
		Log.i("Test", "onSingleTapUp");
		if ((mHideContainer.getVisibility() == View.GONE)
				|| (mHideContainer.getVisibility() == View.INVISIBLE)) {
			if (seekUpdater != null) {
				seekUpdater.startIt();
				seekUpdater.refresh();
			}
			mHideContainer.setVisibility(View.VISIBLE);

		} else {
			mHideContainer.setVisibility(View.INVISIBLE);
			if (seekUpdater != null)
				seekUpdater.stopIt();
		}
		return false;
	}
}
