package com.scut.hujie.k210nettyandroid.bean;

import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@NoArgsConstructor
public class K210Msg {
    // K210返回消息是否完整
    private boolean isMsgOk;
    // 返回消息类型
    private byte retMsgType;
    // 是否检测到目标
    private boolean isDetectObject;
    // 是否检测到车
    private boolean isDetectCar;
    // 是否检测到人
    private boolean isDetectPerson;
    // 前方障碍物距离
    private int distance;
    // 舵机是否执行动作
    private boolean isServoMove;
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
                ", direction=" + retMsgType +
                ", is_detect_object=" + isDetectObject +
                ", is_detect_car=" + isDetectCar +
                ", is_detect_person=" + isDetectPerson +
                ", distance=" + distance +
                '}';
    }
}
