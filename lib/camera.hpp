using namespace std;

namespace camerautils
{
class Camera
{
private:
	/* data */
public:
	Camera(/* args */);
	void Init();
	void Snap();
	std::string getName();
	~Camera();
};
}