namespace vpvl2 {

class MockICameraKeyframe : public ICameraKeyframe {
 public:
  MOCK_CONST_METHOD0(clone,
      ICameraKeyframe*());
  MOCK_METHOD0(setDefaultInterpolationParameter,
      void());
  MOCK_METHOD2(setInterpolationParameter,
      void(InterpolationType type, const QuadWord &value));
  MOCK_CONST_METHOD2(getInterpolationParameter,
      void(InterpolationType type, QuadWord &value));
  MOCK_CONST_METHOD0(lookAt,
      Vector3());
  MOCK_CONST_METHOD0(angle,
      Vector3());
  MOCK_CONST_METHOD0(distance,
      Scalar());
  MOCK_CONST_METHOD0(fov,
      Scalar());
  MOCK_CONST_METHOD0(isPerspective,
      bool());
  MOCK_METHOD1(setLookAt,
      void(const Vector3 &value));
  MOCK_METHOD1(setAngle,
      void(const Vector3 &value));
  MOCK_METHOD1(setDistance,
      void(const Scalar &value));
  MOCK_METHOD1(setFov,
      void(const Scalar &value));
  MOCK_METHOD1(setPerspective,
      void(bool value));
};

}  // namespace vpvl2
