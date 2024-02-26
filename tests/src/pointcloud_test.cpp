#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <depthai/depthai.hpp>

#include "depthai-shared/properties/StereoDepthProperties.hpp"
#include "depthai/pipeline/datatype/PointCloudData.hpp"

dai::Pipeline getPipeline(bool sparse) {
    dai::Pipeline pipeline;
    auto monoLeft = pipeline.create<dai::node::MonoCamera>();
    auto monoRight = pipeline.create<dai::node::MonoCamera>();
    auto stereo = pipeline.create<dai::node::StereoDepth>();
    auto pointcloud = pipeline.create<dai::node::PointCloud>();
    auto xout = pipeline.create<dai::node::XLinkOut>();

    monoLeft->setCamera("left");
    monoRight->setCamera("right");

    monoLeft->setResolution(dai::MonoCameraProperties::SensorResolution::THE_720_P);
    monoRight->setResolution(dai::MonoCameraProperties::SensorResolution::THE_720_P);

    stereo->setDefaultProfilePreset(dai::node::StereoDepth::PresetMode::HIGH_DENSITY);
    stereo->setOutputSize(608, 400);

    xout->setStreamName("out");

    pointcloud->initialConfig.setSparse(sparse);

    monoLeft->out.link(stereo->left);
    monoRight->out.link(stereo->right);
    stereo->depth.link(pointcloud->inputDepth);
    pointcloud->outputPointCloud.link(xout->input);

    return pipeline;
}

TEST_CASE("dense pointcloud") {
    dai::Device device(getPipeline(false));

    auto outQ = device.getOutputQueue("out");
    for(int i = 0; i < 10; ++i) {
        auto pcl = outQ->get<dai::PointCloudData>();
        REQUIRE(pcl != nullptr);
        REQUIRE(pcl->getPoints().size() == 608UL * 400UL);
        REQUIRE(pcl->getMinX() < 0);
        REQUIRE(pcl->getMaxX() > 0);
        REQUIRE(pcl->getMinY() < 0);
        REQUIRE(pcl->getMaxY() > 0);
    }
}

TEST_CASE("sparse pointcloud") {
    dai::Device device(getPipeline(true));

    auto outQ = device.getOutputQueue("out");
    for(int i = 0; i < 10; ++i) {
        auto pcl = outQ->get<dai::PointCloudData>();
        REQUIRE(pcl != nullptr);
        REQUIRE(pcl->getPoints().size() <= 608UL * 400UL);
        REQUIRE(pcl->getMinX() < 0);
        REQUIRE(pcl->getMaxX() > 0);
        REQUIRE(pcl->getMinY() < 0);
        REQUIRE(pcl->getMaxY() > 0);
        REQUIRE(pcl->getMinZ() > 0);
    }
}
