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
	void DeInit();
	void Snap(uint8_t *);
	std::string getName();
	~Camera();
};
}