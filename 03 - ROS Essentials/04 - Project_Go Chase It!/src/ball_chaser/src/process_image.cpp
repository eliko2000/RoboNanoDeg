#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    //Request a service and pass the velocities to it to drive the robot

    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
    

    // Call the command_robot service and pass the requested velocities
    if (!client.call(srv))
        ROS_ERROR("Failed to call service command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    uint32_t image_width = img.width;
    uint32_t image_height = img.height;
    uint32_t pixel_index=0;
    uint32_t third_width=image_width/3;
    int left_count=0;
    int right_count=0;
    int center_count=0;

    //  Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    
        // Get image data
    const std::vector<uint8_t>& image_data = img.data;

    // Loop through each pixel in width-first and then height order
    for (int y = 0; y < image_height; ++y)
    {
        for (int x = 0; x < image_width; ++x)
        {
            // Calculate the index of the current pixel in the image data array
            int pixel_index = (y * image_width + x)*3;

            // Check if pixel white
            if (image_data[pixel_index] == white_pixel && image_data[pixel_index+1] == white_pixel && image_data[pixel_index+2]== white_pixel)
            {
		    if (x<third_width)
		    	left_count++;           
		    else if (x>2*third_width)           
		       right_count++;          
		    else
		       center_count++;
            }
        }
    }

    if (left_count+right_count+center_count==0)
       drive_robot(0,0);
    else if (left_count>right_count && left_count>center_count)
    	drive_robot(0, 0.5);            //turn left
    else if (right_count>left_count && right_count>center_count)           
       drive_robot(0, -0.5);           //turn right      
    else
       drive_robot(0.5, 0);            //drive stright


              
}
int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
