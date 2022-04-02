//
// Created by lacie on 31/03/2022.
//

#include "bbtracker.h"

BBTracker::BBTracker(commandLine cmdLine) : pipeline(parser)
{
    // TODO: Init detectNet, pipeline param
}

std::vector<vector<float>> BBTracker::detectframe(cv::Mat frame)
{

    // TODO: convert to detectNet input and get output

    return net->Detect(frame);
}

void BBTracker::DrawData(cv::Mat frame, int framesCounter, double fontScale)
{
    for (const auto &track : m_tracker->tracks)
    {
        if (track->IsRobust(5,                      // Minimal trajectory size
                            0.2f,                   // Minimal ratio raw_trajectory_points / trajectory_lenght
                            cv::Size2f(0.1f, 8.0f)) // Min and max ratio: width / height
        )
        {
            DrawTrack(frame, 1, *track);
            std::string label = track->m_lastRegion.m_type + ": " + std::to_string((int)(track->m_lastRegion.m_confidence * 100)) + " %";
            // std::string label = std::to_string(track->m_trace.m_firstPass) + " | " + std::to_string(track->m_trace.m_secondPass);
            int baseLine = 0;
            cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
            auto rect(track->GetLastRect());
            cv::rectangle(frame, cv::Rect(cv::Point(rect.x, rect.y - labelSize.height), cv::Size(labelSize.width, labelSize.height + baseLine)), cv::Scalar(255, 255, 255), cv::FILLED);
            cv::putText(frame, label, cv::Point(rect.x, rect.y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
        }
    }
}

void DrawCounter(cv::Mat frame, double fontScale, std::map<string, int> &countObjects_LefttoRight, std::map<string, int> &countObjects_RighttoLeft)
{

    // Line
    cv::Point polyLinePoints[1][4];
    polyLinePoints[0][0] = cv::Point(line2_x2, line2_y2);
    polyLinePoints[0][1] = cv::Point(line2_x1, line2_y1);
    polyLinePoints[0][2] = cv::Point(line1_x1, line1_y1);
    polyLinePoints[0][3] = cv::Point(line1_x2, line1_y2);
    const cv::Point *ppt[1] = {polyLinePoints[0]};
    int npt[] = {4};

    cv::fillPoly(frame, ppt, npt, 1, cv::Scalar(0, 255, 255), 8);

    // cv::line( frame, cv::Point( line2_x1, line2_y1 ), cv::Point( line2_x2, line2_y2), cv::Scalar( 120, 220, 0),  3, 8 );

    // Create Counter label
    std::string counterLabel_L = "Count --> : ";
    std::string counterLabel_R = "Count <-- : ";
    for (auto elem : countObjects_LefttoRight)
    {
        counterLabel_L += elem.first + ": " + std::to_string(elem.second) + " | ";
    }
    for (auto elem : countObjects_RighttoLeft)
    {
        counterLabel_R += elem.first + ": " + std::to_string(elem.second) + " | ";
    }

    // Draw counter label
    int baseLine = 0;
    float fontSize = 0.4;
    cv::Size labelSize_LR = cv::getTextSize(counterLabel_L, cv::FONT_HERSHEY_SIMPLEX, fontSize, 1, &baseLine);
    cv::Size labelSize_RL = cv::getTextSize(counterLabel_R, cv::FONT_HERSHEY_SIMPLEX, fontSize, 1, &baseLine);
    cv::rectangle(frame, cv::Rect(cv::Point(0, 400 - 30 - labelSize_LR.height), cv::Size(labelSize_LR.width, labelSize_LR.height + baseLine)), cv::Scalar(255, 255, 255), cv::FILLED);
    cv::rectangle(frame, cv::Rect(cv::Point(0, line2_y1 + 30 - labelSize_LR.height), cv::Size(labelSize_RL.width, labelSize_RL.height + baseLine)), cv::Scalar(255, 255, 255), cv::FILLED);
    cv::putText(frame, counterLabel_L, cv::Point(0, 400 - 30), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(0, 0, 0), 1.5);
    cv::putText(frame, counterLabel_R, cv::Point(0, line2_y1 + 30), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(0, 0, 0), 1.5);
    //  cv::Size labelSize_LR = cv::getTextSize(counterLabel_L, cv::FONT_HERSHEY_SIMPLEX, fontSize, 1, &baseLine);
    //  cv::Size labelSize_RL = cv::getTextSize(counterLabel_R, cv::FONT_HERSHEY_SIMPLEX, fontSize, 1, &baseLine);
    //  cv::rectangle(frame, cv::Rect(cv::Point(10, 50 - 30 - labelSize_LR.height), cv::Size(labelSize_LR.width, labelSize_LR.height + baseLine)), cv::Scalar(255, 255, 255), CV_FILLED);
    //  cv::rectangle(frame, cv::Rect(cv::Point(10, 600 + 30 - labelSize_LR.height), cv::Size(labelSize_RL.width, labelSize_RL.height + baseLine)), cv::Scalar(255, 255, 255), CV_FILLED);
    //  cv::putText(frame, counterLabel_L, cv::Point(10, 50 - 30), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(0, 0, 0),1.5);
    //  cv::putText(frame, counterLabel_R, cv::Point(10, 600 + 30), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(0, 0, 0),1.5);
}

void CounterUpdater(cv::Mat frame, std::map<string, int> &countObjects_LefttoRight, std::map<string, int> &countObjects_RighttoLeft)
{

    // Draw Counter line
    cv::Point polyLinePoints[1][4];
    polyLinePoints[0][0] = cv::Point(line2_x2, line2_y2);
    polyLinePoints[0][1] = cv::Point(line2_x1, line2_y1);
    polyLinePoints[0][2] = cv::Point(line1_x1, line1_y1);
    polyLinePoints[0][3] = cv::Point(line1_x2, line1_y2);
    const cv::Point *ppt[1] = {polyLinePoints[0]};
    int npt[] = {4};

    cv::fillPoly(frame, ppt, npt, 1, cv::Scalar(0, 255, 255), 8);

    for (const auto &track : m_tracker->tracks)
    {
        if (track->m_trace.size() >= 2)
        {
            // Extract the last two points from trace.
            track_t pt1_x = track->m_trace.at(track->m_trace.size() - 2).m_prediction.x;
            track_t pt1_y = track->m_trace.at(track->m_trace.size() - 2).m_prediction.y;
            track_t pt2_x = track->m_trace.at(track->m_trace.size() - 1).m_prediction.x;
            track_t pt2_y = track->m_trace.at(track->m_trace.size() - 1).m_prediction.y;

            float pt1_position_line1 = (line1_y2 - line1_y1) * pt1_x + (line1_x1 - line1_x2) * pt1_y + (line1_x2 * line1_y1 - line1_x1 * line1_y2);
            float pt2_position_line1 = (line1_y2 - line1_y1) * pt2_x + (line1_x1 - line1_x2) * pt2_y + (line1_x2 * line1_y1 - line1_x1 * line1_y2);
            float pt1_position_line2 = (line2_y2 - line2_y1) * pt1_x + (line2_x1 - line2_x2) * pt1_y + (line2_x2 * line2_y1 - line2_x1 * line2_y2);
            float pt2_position_line2 = (line2_y2 - line2_y1) * pt2_x + (line2_x1 - line2_x2) * pt2_y + (line2_x2 * line2_y1 - line2_x1 * line2_y2);
            if (310 <= pt2_y and pt2_y <= 330)
            {
                cv::fillPoly(frame, ppt, npt, 1, cv::Scalar(0, 100, 0), 8);
            }

            // Update Counter from Left to Right
            if (direction == 0)
            {
                if (pt1_position_line1 < 0 && pt2_position_line1 >= 0)
                {
                    track->m_trace.FirstPass();
                }
                if (track->m_trace.GetFirstPass() && pt2_position_line2 >= 0 && !track->m_trace.GetSecondPass())
                {
                    track->m_trace.SecondPass();
                    std::pair<std::map<string, int>::iterator, bool> ret;
                    ret = countObjects_LefttoRight.insert(std::pair<string, int>(track->m_lastRegion.m_type, 1));
                    if (!ret.second)
                    {
                        ret.first->second = ret.first->second + 1;
                    }
                }
                // Update Counter from Right to Left
            }
            else if (direction == 1)
            {
                if (pt2_position_line2 <= 0 && pt1_position_line2 > 0)
                {
                    track->m_trace.FirstPass();
                }
                if (track->m_trace.GetFirstPass() && pt2_position_line1 <= 0 && !track->m_trace.GetSecondPass())
                {
                    track->m_trace.SecondPass();
                    std::pair<std::map<string, int>::iterator, bool> ret;
                    ret = countObjects_RighttoLeft.insert(std::pair<string, int>(track->m_lastRegion.m_type, 1));
                    if (!ret.second)
                    {
                        ret.first->second = ret.first->second + 1;
                    }
                }
            } // Update Counter from both directions
            else
            {
                if (pt2_position_line2 <= 0 && pt1_position_line2 > 0)
                {
                    track->m_trace.FirstPass();
                    track->m_trace.m_directionFromLeft = true;
                }
                else if (pt1_position_line1 < 0 && pt2_position_line1 >= 0)
                {
                    track->m_trace.FirstPass();
                    track->m_trace.m_directionFromLeft = false;
                }
                if (track->m_trace.GetFirstPass() && pt2_position_line1 <= 0 && !track->m_trace.GetSecondPass() && track->m_trace.m_directionFromLeft)
                {
                    track->m_trace.SecondPass();
                    std::pair<std::map<string, int>::iterator, bool> ret;
                    ret = countObjects_RighttoLeft.insert(std::pair<string, int>(track->m_lastRegion.m_type, 1));
                    if (!ret.second)
                    {
                        ret.first->second = ret.first->second + 1;
                    }
                }
                else if (track->m_trace.GetFirstPass() && pt2_position_line2 >= 0 && !track->m_trace.GetSecondPass() && !track->m_trace.m_directionFromLeft)
                {
                    track->m_trace.SecondPass();
                    std::pair<std::map<string, int>::iterator, bool> ret;
                    ret = countObjects_LefttoRight.insert(std::pair<string, int>(track->m_lastRegion.m_type, 1));
                    if (!ret.second)
                    {
                        ret.first->second = ret.first->second + 1;
                    }
                }
            }
        }
    }
}
