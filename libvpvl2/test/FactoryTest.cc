#include "Common.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/extensions/icu4c/Encoding.h"
#include "vpvl2/asset/Model.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/mvd/BoneKeyframe.h"
#include "vpvl2/mvd/CameraKeyframe.h"
#include "vpvl2/mvd/LightKeyframe.h"
#include "vpvl2/mvd/MorphKeyframe.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/vmd/Motion.h"
#include "vpvl2/vmd/BoneKeyframe.h"
#include "vpvl2/vmd/CameraKeyframe.h"
#include "vpvl2/vmd/LightKeyframe.h"
#include "vpvl2/vmd/MorphKeyframe.h"
#include "mock/Bone.h"
#include "mock/Model.h"
#include "mock/Morph.h"

#ifdef VPVL2_LINK_VPVL
#include "vpvl2/pmd/Model.h"
#else
#include "vpvl2/pmd2/Model.h"
#endif

using namespace ::testing;
using namespace std::tr1;
using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;

TEST(FactoryTest, CreateEmptyModels)
{
    Encoding encoding(0);
    Factory factory(&encoding);
    QScopedPointer<IModel> pmd(factory.newModel(IModel::kPMDModel));
#ifdef VPVL2_LINK_VPVL
    ASSERT_TRUE(dynamic_cast<pmd::Model *>(pmd.data()));
#else
    ASSERT_TRUE(dynamic_cast<pmd2::Model *>(pmd.data()));
#endif
    QScopedPointer<IModel> pmx(factory.newModel(IModel::kPMXModel));
    ASSERT_TRUE(dynamic_cast<pmx::Model *>(pmx.data()));
    QScopedPointer<IModel> asset(factory.newModel(IModel::kAssetModel));
    ASSERT_TRUE(dynamic_cast<asset::Model *>(asset.data()));
}

TEST(FactoryTest, CreateEmptyMotions)
{
    Encoding encoding(0);
    Factory factory(&encoding);
    MockIModel model;
    QScopedPointer<IMotion> vmd(factory.newMotion(IMotion::kVMDMotion, &model));
    ASSERT_TRUE(dynamic_cast<vmd::Motion *>(vmd.data()));
    QScopedPointer<IMotion> mvd(factory.newMotion(IMotion::kMVDMotion, &model));
    ASSERT_TRUE(dynamic_cast<mvd::Motion *>(mvd.data()));
}

TEST(FactoryTest, CreateEmptyBoneKeyframes)
{
    Encoding encoding(0);
    Factory factory(&encoding);
    MockIModel model;
    ASSERT_FALSE(factory.createBoneKeyframe(0));
    QScopedPointer<IMotion> vmd(factory.newMotion(IMotion::kVMDMotion, &model));
    QScopedPointer<IBoneKeyframe> vbk(factory.createBoneKeyframe(vmd.data()));
    ASSERT_TRUE(dynamic_cast<vmd::BoneKeyframe *>(vbk.data()));
    QScopedPointer<IMotion> mvd(factory.newMotion(IMotion::kMVDMotion, &model));
    QScopedPointer<IBoneKeyframe> mbk(factory.createBoneKeyframe(mvd.data()));
    ASSERT_TRUE(dynamic_cast<mvd::BoneKeyframe *>(mbk.data()));
}

TEST(FactoryTest, CreateEmptyCameraKeyframes)
{
    Encoding encoding(0);
    Factory factory(&encoding);
    MockIModel model;
    ASSERT_FALSE(factory.createCameraKeyframe(0));
    QScopedPointer<IMotion> vmd(factory.newMotion(IMotion::kVMDMotion, &model));
    QScopedPointer<ICameraKeyframe> vck(factory.createCameraKeyframe(vmd.data()));
    ASSERT_TRUE(dynamic_cast<vmd::CameraKeyframe *>(vck.data()));
    QScopedPointer<IMotion> mvd(factory.newMotion(IMotion::kMVDMotion, &model));
    QScopedPointer<ICameraKeyframe> mck(factory.createCameraKeyframe(mvd.data()));
    ASSERT_TRUE(dynamic_cast<mvd::CameraKeyframe *>(mck.data()));
}

TEST(FactoryTest, CreateEmptyLightKeyframes)
{
    Encoding encoding(0);
    Factory factory(&encoding);
    MockIModel model;
    ASSERT_FALSE(factory.createLightKeyframe(0));
    QScopedPointer<IMotion> vmd(factory.newMotion(IMotion::kVMDMotion, &model));
    QScopedPointer<ILightKeyframe> vlk(factory.createLightKeyframe(vmd.data()));
    ASSERT_TRUE(dynamic_cast<vmd::LightKeyframe *>(vlk.data()));
    QScopedPointer<IMotion> mvd(factory.newMotion(IMotion::kMVDMotion, &model));
    QScopedPointer<ILightKeyframe> mlk(factory.createLightKeyframe(mvd.data()));
    ASSERT_TRUE(dynamic_cast<mvd::LightKeyframe *>(mlk.data()));
}

TEST(FactoryTest, CreateEmptyMorphKeyframes)
{
    Encoding encoding(0);
    Factory factory(&encoding);
    MockIModel model;
    ASSERT_FALSE(factory.createMorphKeyframe(0));
    QScopedPointer<IMotion> vmd(factory.newMotion(IMotion::kVMDMotion, &model));
    QScopedPointer<IMorphKeyframe> vmk(factory.createMorphKeyframe(vmd.data()));
    ASSERT_TRUE(dynamic_cast<vmd::MorphKeyframe *>(vmk.data()));
    QScopedPointer<IMotion> mvd(factory.newMotion(IMotion::kMVDMotion, &model));
    QScopedPointer<IMorphKeyframe> mmk(factory.createMorphKeyframe(mvd.data()));
    ASSERT_TRUE(dynamic_cast<mvd::MorphKeyframe *>(mmk.data()));
}

class FactoryModelTest : public TestWithParam<IModel::Type> {};

TEST_P(FactoryModelTest, StopInfiniteParentModelLoop)
{
    Encoding encoding(0);
    Factory factory(&encoding);
    IModel::Type type = GetParam();
    QScopedPointer<IModel> model1(factory.newModel(type)),
            model2(factory.newModel(type)),
            model3(factory.newModel(type));
    model3->setParentModelRef(model2.data());
    model2->setParentModelRef(model1.data());
    model1->setParentModelRef(model3.data());
    ASSERT_EQ(model2.data(), model3->parentModelRef());
    ASSERT_EQ(model1.data(), model2->parentModelRef());
    ASSERT_EQ(0, model1->parentModelRef());
}

class MotionConversionTest : public TestWithParam< tuple<QString, IMotion::Type > > {};

ACTION_P(FindBone, bones)
{
    MockIBone *bone = new MockIBone();
    EXPECT_CALL(*bone, name()).Times(AnyNumber()).WillRepeatedly(Return(arg0));
    (*bones)->append(bone);
    return bone;
}

ACTION_P(FindMorph, morphs)
{
    MockIMorph *morph = new MockIMorph();
    EXPECT_CALL(*morph, name()).Times(AnyNumber()).WillRepeatedly(Return(arg0));
    (*morphs)->append(morph);
    return morph;
}

TEST_P(MotionConversionTest, ConvertModelMotion)
{
    QFile file("motion." + get<0>(GetParam()));
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding(0);
        Factory factory(&encoding);
        MockIModel model;
        QScopedPointer<Array<IBone *>, ScopedPointerListDeleter> bones(new Array<IBone *>);
        QScopedPointer<Array<IMorph *>, ScopedPointerListDeleter> morphs(new Array<IMorph *>);
        EXPECT_CALL(model, findBoneRef(_)).Times(AtLeast(1)).WillRepeatedly(FindBone(&bones));
        EXPECT_CALL(model, findMorphRef(_)).Times(AtLeast(1)).WillRepeatedly(FindMorph(&morphs));
        bool ok;
        QScopedPointer<IMotion> source(factory.createMotion(data, size, &model, ok));
        ASSERT_TRUE(ok);
        IMotion::Type type = get<1>(GetParam());
        QScopedPointer<IMotion> dest(factory.convertMotion(source.data(), type));
        ASSERT_EQ(dest->type(), type);
        const int nbkeyframes = source->countKeyframes(IKeyframe::kBoneKeyframe);
        const int nmkeyframes = source->countKeyframes(IKeyframe::kMorphKeyframe);
        ASSERT_EQ(nbkeyframes, dest->countKeyframes(IKeyframe::kBoneKeyframe));
        ASSERT_EQ(nmkeyframes, dest->countKeyframes(IKeyframe::kMorphKeyframe));
        for (int i = 0; i < nbkeyframes; i++) {
            ASSERT_TRUE(CompareBoneKeyframe(*source->findBoneKeyframeRefAt(i), *dest->findBoneKeyframeRefAt(i)));
        }
        for (int i = 0; i < nmkeyframes; i++) {
            ASSERT_TRUE(CompareMorphKeyframe(*source->findMorphKeyframeRefAt(i), *dest->findMorphKeyframeRefAt(i)));
        }
    }
}

TEST_P(MotionConversionTest, ConvertCameraMotion)
{
    QFile file("camera." + get<0>(GetParam()));
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding(0);
        Factory factory(&encoding);
        bool ok;
        QScopedPointer<IMotion> source(factory.createMotion(data, size, 0, ok));
        ASSERT_TRUE(ok);
        IMotion::Type type = get<1>(GetParam());
        QScopedPointer<IMotion> dest(factory.convertMotion(source.data(), type));
        ASSERT_EQ(dest->type(), type);
        const int nckeyframes = source->countKeyframes(IKeyframe::kCameraKeyframe);
        ASSERT_EQ(nckeyframes, dest->countKeyframes(IKeyframe::kCameraKeyframe));
        for (int i = 0; i < nckeyframes; i++) {
            ASSERT_TRUE(CompareCameraKeyframe(*source->findCameraKeyframeRefAt(i), *dest->findCameraKeyframeRefAt(i)));
        }
    }
}

INSTANTIATE_TEST_CASE_P(FactoryInstance, FactoryModelTest, Values(IModel::kAssetModel, IModel::kPMDModel, IModel::kPMXModel));
INSTANTIATE_TEST_CASE_P(FactoryInstance, MotionConversionTest,
                        Combine(Values("vmd", "mvd"), Values(IMotion::kVMDMotion, IMotion::kMVDMotion)));
