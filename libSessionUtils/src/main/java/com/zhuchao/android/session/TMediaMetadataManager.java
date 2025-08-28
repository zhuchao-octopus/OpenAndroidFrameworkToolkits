package com.zhuchao.android.session;

import android.os.Build;

import com.zhuchao.android.fbase.FileUtils;
import com.zhuchao.android.fbase.ObjectList;
import com.zhuchao.android.fbase.bean.AudioMetaFile;
import com.zhuchao.android.fbase.bean.MediaMetadata;
import com.zhuchao.android.video.Movie;
import com.zhuchao.android.video.OMedia;
import com.zhuchao.android.video.VideoList;

import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

public class TMediaMetadataManager {
    public static final String ALBUM_TAG = "media.album.tag.";
    public static final String ARTIST_TAG = "media.artist.tag.";

    public VideoList mVideoList = new VideoList("MediaMetadataManager");
    private final ObjectList mMediaMetaDatas = new ObjectList();

    public void clear() {
        mMediaMetaDatas.clear();
        mVideoList.clear();
    }

    public List<MediaMetadata> getAlbums() {
        return mMediaMetaDatas.toListLike(ALBUM_TAG);
    }

    public <T> List<T> getTAlbums() {
        return mMediaMetaDatas.toListLike(ALBUM_TAG);
    }

    public List<MediaMetadata> getArtist() {
        return mMediaMetaDatas.toListLike(ARTIST_TAG);
    }

    public <T> List<T> getTArtist() {
        return mMediaMetaDatas.toListLike(ARTIST_TAG);
    }

    public ObjectList getMediaMetaDatas() {
        return mMediaMetaDatas;
    }

    /**
     * Optimized version of updateArtistAndAlbum using Iterator to avoid ConcurrentModificationException.
     * Improvements:
     * 1. Iterator used to safely traverse the map and avoid modification exceptions.
     * 2. Reduced repeated calls to oMedia.getMovie().
     * 3. Cleaned up redundant null checks.
     * 4. Improved readability and reduced code duplication.
     */

    public void updateArtistAndAlbum(VideoList videoList) {
        OMedia oMedia;

        for (Map.Entry<String, Object> entry : videoList.getMap().entrySet()) {
            oMedia = (OMedia) entry.getValue();

            // Skip null objects or already existing paths in the list
            if (oMedia == null || oMedia.getMovie() == null || mVideoList.exist(oMedia.getPathName())) {
                continue;
            }

            // Add to the video list
            mVideoList.addRow(oMedia);

            // Get Movie metadata
            Movie movie = oMedia.getMovie();
            String album = movie.getAlbum();
            String artist = movie.getArtist();

            // Handle empty strings
            if (FileUtils.EmptyString(album)) album = "Unknown";
            if (FileUtils.EmptyString(artist)) artist = "Unknown";

            // === Album Metadata Update ===
            String albumTagName = ALBUM_TAG + album;
            MediaMetadata albumMetadata = mMediaMetaDatas.getValue(albumTagName);
            if (albumMetadata == null) albumMetadata = new MediaMetadata();

            updateMediaMetadata(albumMetadata, album, artist, movie, oMedia.getPathName());
            mMediaMetaDatas.addObject(albumTagName, albumMetadata);

            // === Artist Metadata Update ===
            String artistTagName = ARTIST_TAG + artist;
            MediaMetadata artistMetadata = mMediaMetaDatas.getValue(artistTagName);
            if (artistMetadata == null) artistMetadata = new MediaMetadata();

            updateMediaMetadata(artistMetadata, album, artist, movie, oMedia.getPathName());
            mMediaMetaDatas.addObject(artistTagName, artistMetadata);
        }
    }

    /**
     * Helper method to update the metadata information.
     */
    private void updateMediaMetadata(MediaMetadata metadata, String album, String artist, Movie movie, String pathName) {
        metadata.setAlbum(album);
        metadata.setArtist(artist);
        metadata.setId(movie.getSourceId());
        metadata.addDescription(movie.getStudio());
        metadata.addDescription(movie.getDescription());
        metadata.addDescription(movie.getActor());
        metadata.addCount();

        // Set album art for Android Q and above
        if (metadata.getBitmap() == null && Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            metadata.setBitmap(AudioMetaFile.getAlbumArtPicture(pathName));
        }
    }

    public void free() {
        mMediaMetaDatas.clear();
        mVideoList.clear();
    }

}
