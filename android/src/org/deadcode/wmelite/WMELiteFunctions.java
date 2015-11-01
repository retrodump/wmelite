package org.deadcode.wmelite;

import java.nio.charset.Charset;
import java.nio.charset.IllegalCharsetNameException;
import java.nio.charset.UnsupportedCharsetException;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Handler;
import android.os.Environment;
import android.os.storage.OnObbStateChangeListener;
import android.os.storage.StorageManager;

import android.app.*;

// import com.purplebrain.adbuddiz.sdk.*;

public class WMELiteFunctions {

	private Context c;
	private StorageManager stMgr;
	private ObbMountListener mountListener;
	private String obbMountPathOverride;
	private Handler mainThreadRunHandler;
	private Activity act;
	
	private String appPackageName;
	private int    appVersionCode;
	
	public WMELiteFunctions() {
		obbMountPathOverride = null;
	}
	
	public boolean getStorageCallbackRequired() {
		return getObbSourcePath() != null;
	}
	
	public void setActivity(Activity act)
	{
		this.act = act;
	}
	
	public boolean setContext(Context c, Handler mainThreadRunHandler) {
		this.c = c;
		
		// check whether we need the storage manager
		if (getStorageCallbackRequired() == true)
		{
			appPackageName = c.getPackageName();
			PackageInfo pi;
			try {
				pi = c.getPackageManager().getPackageInfo(appPackageName, PackageManager.GET_META_DATA);
				appVersionCode = pi.versionCode;
			} catch (NameNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			this.mainThreadRunHandler = mainThreadRunHandler;
			stMgr = (StorageManager) c.getSystemService(Context.STORAGE_SERVICE);
			mountListener = new ObbMountListener();
			String rawPath = getObbSourcePath().substring(11);
			boolean success = stMgr.mountObb(rawPath, null, mountListener);
			if (success == false)
			{
				System.err.println("Initiating OBB file mount failed!");
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	
	public void init() {
		nativeInit(c.getAssets());
	}
	
	public void exit() {
		// check whether we need the storage manager
		if (getStorageCallbackRequired() == true)
		{
			String rawPath = getObbSourcePath().substring(11);
			boolean success = stMgr.unmountObb(rawPath, false, mountListener);
			if (success == false)
			{
				System.err.println("Initiating OBB file unmount failed!");
			}
		}		
	}
		
	private native void nativeInit(AssetManager manager);
	
	public String getLogFileDirectory() {
		// this assumed the "external" storage exists and is mounted r/w
		// return Environment.getExternalStorageDirectory().getAbsolutePath();
		
		return "/mnt/sdcard/";
	}
	
	public String getPrivateFilesPath() {
		// this returns the app-private storage for user data
		return c.getFilesDir().getAbsolutePath();
	}

	public String getDeviceTypeHint() {
		if (c != null) {
			return ((c.getResources().getConfiguration().screenLayout
	            & Configuration.SCREENLAYOUT_SIZE_MASK)
	            >= Configuration.SCREENLAYOUT_SIZE_LARGE) ? "tablet" : "phone";
		} else {
			return "tablet";
		}
	}

	public String getGamePackagePath() {
		// change to fit your needs, maybe to point to the expansion files downloaded from Google play
		// return Environment.getExternalStorageDirectory().getAbsolutePath();


		/*		
		if (obbMountPathOverride != null) {
			return obbMountPathOverride;
		}
		
		System.out.println("getGamePackagePath() called while OBB not mounted!!! This is probably wrong!");
		
		return getObbSourcePath();
		*/
		
		return "/mnt/sdcard/";
	}

	// return NULL if not using OBB mount (!)
	private String getObbSourcePath()
	{
		/*
		return "obbmount://" 
			+ Environment.getExternalStorageDirectory() 
			+ "/Android/obb/" 
			+ appPackageName 
			+ "/" 
			+ "main." 
			+ appVersionCode 
			+ "." 
			+ appPackageName
			+ ".obb";
			*/

		return null;
	}

	public String getGameFilePath() {
		// place the files here that regular WME does not package, i.e. fonts                        
		//		return "asset://raw";

		return "/mnt/sdcard/";
		// return "/mnt/sdcard/demo/";
	}

	public String getFontPath() {
		if (c != null) {
			// FIXME is this correct?
			return "asset://raw";
		} else {
			return ".";
		}
	}
	
	public String getLocalSettingsPath() {
		// for development and debugging, a local settings.xml
		// can be placed onto the device using a location
		// that can be accessed by both wmelite and the user
		
		// return "asset://raw";

		return "/mnt/sdcard/";
		// return "/mnt/sdcard/demo/";
	}
	
	public byte[] getEncodedString(String inputString, String charsetName) {
		
		// change the default here if all the strings used in the project have the same encoding
		Charset charset = Charset.forName("US-ASCII");
		
		if (charsetName != null) {
			try {
				charset = Charset.forName(charsetName);
			} catch (IllegalCharsetNameException e1) {
				System.err.println("Charset name " + charsetName + " is illegal, using default!");
			} catch (UnsupportedCharsetException e2) {
				System.err.println("Charset name " + charsetName + " is not supported, using default!");
			}
		}
		
		// System.out.println("Encoding string '" + inputString + "' to charset " + charset.name());
		
		byte[] res = inputString.getBytes(charset);
		
		// System.out.println("Input len=" + inputString.length() + " to output byte array len=" + res.length);
		
		return res; 
	}
	
	public String getUTFString(byte[] inputBytes, String charsetName) {
		// change the default here if all the strings used in the project have the same encoding
		Charset charset = Charset.forName("US-ASCII");
		
		if (charsetName != null) {
			try {
				charset = Charset.forName(charsetName);
			} catch (IllegalCharsetNameException e1) {
				System.err.println("Charset name " + charsetName + " is illegal, using default!");
			} catch (UnsupportedCharsetException e2) {
				System.err.println("Charset name " + charsetName + " is not supported, using default!");
			}
		}
		
		String res = new String(inputBytes, charset);
		
		return res;
	}
	
	
	public void showURLInBrowser(String urlToShow)
	{
	  Intent myIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(urlToShow));
	  c.startActivity(myIntent);
	}

	public class ObbMountListener extends OnObbStateChangeListener
	{
		public void onObbStateChange(String path, int state)
		{
			if ((state == ERROR_ALREADY_MOUNTED) || (state == MOUNTED))
			{
				// path is now accessible
				System.out.println("OBB mount succeeded!");
				obbMountPathOverride = "obbmount://" + stMgr.getMountedObbPath(path);
				System.out.println("Internal OBB mount path: " + obbMountPathOverride);
				mainThreadRunHandler.sendEmptyMessage(1);
			}
			else if ((state == ERROR_NOT_MOUNTED) || (state == UNMOUNTED))
			{
				System.out.println("OBB unmount succeeded!");
				obbMountPathOverride = null;
			}
			else
			{
				System.err.println("OBB mount error: " + state);
			}
		}
	}
	
	public int advertisementPrepare(String key, int number)
	{
		System.out.println("Query Advertisement SDK to prepare ads/check if ads prepared. Key=" + key + ",number=" + number + ".");
		
		// place appropriate code here
		// if (AdBuddiz.isReadyToShowAd(act)) { // this = current Activity
		// 	return 1;
		// }
		
		return 0;
	}
	
	public int advertisementShow(String key, int number)
	{
		System.out.println("Instruct Advertisement SDK to show an ad. Key=" + key + ",number=" + number + ".");
		
		// place appropriate code here
		// AdBuddiz.showAd(act);
		
		return 0;
	}

}

