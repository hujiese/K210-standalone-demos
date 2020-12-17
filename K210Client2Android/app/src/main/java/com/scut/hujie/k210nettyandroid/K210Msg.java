package com.scut.hujie.k210nettyandroid;

import lombok.Data;

@Data
public class K210Msg {

    private boolean isMsgOk;
    // 检测方向
    private byte direction;
    // 是否检测到目标
    private boolean is_detect_object;
    // 是否检测到车
    private boolean is_detect_car;
    // 是否检测到人
    private boolean is_detect_person;
    // 前方障碍物距离
    private int distance;
    // 拍摄图片
    private byte[] jpegImage;

    public K210Msg(int imageLen) {
        jpegImage = new byte[imageLen];
    }

    public void copyImage(Object src, int srcPos, int lentth) {
        System.arraycopy(src, srcPos, jpegImage, 0, lentth);
    }

    @Override
    public String toString() {
        return "K210Msg{" +
                "isMsgOk=" + isMsgOk +
                ", direction=" + direction +
                ", is_detect_object=" + is_detect_object +
                ", is_detect_car=" + is_detect_car +
                ", is_detect_person=" + is_detect_person +
                ", distance=" + distance +
                '}';
    }
}
