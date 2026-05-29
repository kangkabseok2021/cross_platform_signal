#ifndef CAMERA_SIMULATOR_H
#define CAMERA_SIMULATOR_H

class CameraSimulator {
private:
    int current_frame_id_;
public:
    CameraSimulator(double fps = 30.0);
    int next_frame();
    int get_current_frame_id() const;
};

#endif // CAMERA_SIMULATOR_H
