#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkProperty.h>
#include <vtkWarpScalar.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkCommand.h>

const float VISUAL_SCALE = 0.01f;
const float SOFTENING_FACTOR = 0.5f;

class vtkTimerCallback : public vtkCommand
{
public:
    static vtkTimerCallback* New() {
        return new vtkTimerCallback;
    }

    void Execute(vtkObject* caller, unsigned long eventId, void* callData) override {
        double time = this->TimerCount * 0.1;
        this->SphereActor->SetPosition(cos(time) * 10.0, 1.0, sin(time) * 10.0);
        
        this->UpdateGridDeformation();

        vtkRenderWindowInteractor* iren = static_cast<vtkRenderWindowInteractor*>(caller);
        iren->GetRenderWindow()->Render();
        this->TimerCount++;
    }

    void UpdateGridDeformation() {
        double spherePos[3];
        this->SphereActor->GetPosition(spherePos);

        vtkPolyData* planeData = static_cast<vtkPolyData*>(this->PlaneSource->GetOutput());
        vtkSmartPointer<vtkDoubleArray> scalars = vtkSmartPointer<vtkDoubleArray>::New();
        scalars->SetNumberOfComponents(1);
        scalars->SetName("DeformationScalars");

        for (vtkIdType i = 0; i < planeData->GetNumberOfPoints(); i++)
        {
            double p[3];
            planeData->GetPoint(i, p);

            float dx = p[0] - spherePos[0];
            float dz = p[2] - spherePos[2];
            float rSq = dx * dx + dz * dz;

            double potential = -this->GravitationalParameter / sqrt(rSq + SOFTENING_FACTOR * SOFTENING_FACTOR);
            scalars->InsertNextValue(potential);
        }

        planeData->GetPointData()->SetScalars(scalars);
        this->WarpFilter->Update();
    }

    vtkActor* SphereActor;
    vtkPlaneSource* PlaneSource;
    vtkWarpScalar* WarpFilter;
    float GravitationalParameter;

private:
    int TimerCount = 0;
};

int main(int, char*[])
{
    vtkSmartPointer<vtkNamedColors> colors = vtkSmartPointer<vtkNamedColors>::New();

    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetRadius(2.0);
    sphereSource->SetPhiResolution(30);
    sphereSource->SetThetaResolution(30);
    
    vtkSmartPointer<vtkPlaneSource> planeSource = vtkSmartPointer<vtkPlaneSource>::New();
    planeSource->SetXResolution(100);
    planeSource->SetYResolution(100);
    planeSource->SetOrigin(-50, 0, -50);
    planeSource->SetPoint1(50, 0, -50);
    planeSource->SetPoint2(-50, 0, 50);
    planeSource->Update();

    vtkSmartPointer<vtkWarpScalar> warpScalar = vtkSmartPointer<vtkWarpScalar>::New();
    warpScalar->SetInputData(planeSource->GetOutput());
    warpScalar->SetScaleFactor(VISUAL_SCALE);
    warpScalar->UseNormalOn();
    warpScalar->SetNormal(0, 1, 0);

    vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    sphereMapper->SetInputConnection(sphereSource->GetOutputPort());

    vtkSmartPointer<vtkPolyDataMapper> gridMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    gridMapper->SetInputConnection(warpScalar->GetOutputPort());

    vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
    sphereActor->SetMapper(sphereMapper);
    sphereActor->GetProperty()->SetColor(colors->GetColor3d("Silver").GetData());

    vtkSmartPointer<vtkActor> gridActor = vtkSmartPointer<vtkActor>::New();
    gridActor->SetMapper(gridMapper);
    gridActor->GetProperty()->SetRepresentationToWireframe();
    gridActor->GetProperty()->SetColor(colors->GetColor3d("White").GetData());

    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(sphereActor);
    renderer->AddActor(gridActor);
    renderer->SetBackground(colors->GetColor3d("DarkSlateBlue").GetData());

    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(1280, 720);
    renderWindow->SetWindowName("Simulador de Gravidade com VTK");

    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);
    renderWindowInteractor->Initialize();

    vtkSmartPointer<vtkTimerCallback> timerCallback = vtkSmartPointer<vtkTimerCallback>::New();
    timerCallback->SphereActor = sphereActor;
    timerCallback->PlaneSource = planeSource;
    timerCallback->WarpFilter = warpScalar;
    timerCallback->GravitationalParameter = 400.0f;
    
    timerCallback->UpdateGridDeformation();

    renderWindowInteractor->AddObserver(vtkCommand::TimerEvent, timerCallback);
    renderWindowInteractor->CreateRepeatingTimer(1000.0 / 60.0);

    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}