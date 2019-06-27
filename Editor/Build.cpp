#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <regex>
#include "vendor/stb_image.h"
#include "vendor/stb_image_resize.h"
#include "vendor/stb_image_write.h"
#include "build.h"
#include "util.h"
#include "debug.h"

namespace UltraEd
{
    bool CBuild::WriteSpecFile(vector<CActor*> actors)
    {
        string specSegments, specIncludes;
        const char *specHeader = "#include <nusys.h>\n\n"
            "beginseg"
            "\n\tname \"code\""
            "\n\tflags BOOT OBJECT"
            "\n\tentry nuBoot"
            "\n\taddress NU_SPEC_BOOT_ADDR"
            "\n\tstack NU_SPEC_BOOT_STACK"
            "\n\tinclude \"codesegment.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\rspboot.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\aspMain.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspF3DEX2.fifo.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspL3DEX2.fifo.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspF3DEX2.Rej.fifo.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspF3DEX2.NoN.fifo.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspF3DLX2.Rej.fifo.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspS2DEX2.fifo.o\""
            "\nendseg\n";
        const char *specIncludeStart = "\nbeginwave"
            "\n\tname \"main\""
            "\n\tinclude \"code\"";
        const char *specIncludeEnd = "\nendwave";

        int loopCount = 0;
        for (auto actor : actors)
        {
            if (actor->GetType() != ActorType::Model) continue;

            string newResName = CUtil::NewResourceName(loopCount++);
            string id = CUtil::GuidToString(actor->GetId());
            id.insert(0, CUtil::RootPath().append("\\"));
            id.append(".rom.sos");

            string modelName(newResName);
            modelName.append("_M");

            specSegments.append("\nbeginseg\n\tname \"");
            specSegments.append(modelName);
            specSegments.append("\"\n\tflags RAW\n\tinclude \"");
            specSegments.append(id);
            specSegments.append("\"\nendseg\n");

            specIncludes.append("\n\tinclude \"");
            specIncludes.append(modelName);
            specIncludes.append("\"");

            map<string, string> resources = actor->GetResources();
            if (resources.count("textureDataPath"))
            {
                // Load the set texture and resize to required dimensions.
                string path = resources["textureDataPath"];
                int width, height, channels;
                unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 3);
                if (data)
                {
                    // Force 32 x 32 texture for now.
                    if (stbir_resize_uint8(data, width, height, 0, data, 32, 32, 0, 3))
                    {
                        path.append(".rom.png");
                        stbi_write_png(path.c_str(), 32, 32, 3, data, 0);
                    }

                    stbi_image_free(data);
                }

                string textureName(newResName);
                textureName.append("_T");

                specSegments.append("\nbeginseg\n\tname \"");
                specSegments.append(textureName);
                specSegments.append("\"\n\tflags RAW\n\tinclude \"");
                specSegments.append(path);
                specSegments.append("\"\nendseg\n");

                specIncludes.append("\n\tinclude \"");
                specIncludes.append(textureName);
                specIncludes.append("\"");
            }
        }

        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string specPath(buffer);
            specPath.append("\\..\\..\\Engine\\spec");
            FILE *file = fopen(specPath.c_str(), "w");
            if (file == NULL) return false;

            string slashesNormalized(specHeader);
            slashesNormalized = regex_replace(slashesNormalized, regex("\\\\"), "\\\\");
            fwrite(slashesNormalized.c_str(), 1, slashesNormalized.size(), file);

            slashesNormalized = string(specSegments);
            slashesNormalized = regex_replace(slashesNormalized, regex("\\\\"), "\\\\");
            fwrite(slashesNormalized.c_str(), 1, slashesNormalized.size(), file);

            fwrite(specIncludeStart, 1, strlen(specIncludeStart), file);
            fwrite(specIncludes.c_str(), 1, specIncludes.size(), file);
            fwrite(specIncludeEnd, 1, strlen(specIncludeEnd), file);
            fclose(file);
            return true;
        }
        return false;
    }

    bool CBuild::WriteSegmentsFile(vector<CActor*> actors)
    {
        string romSegments;
        int loopCount = 0;
        for (auto actor : actors)
        {
            if (actor->GetType() != ActorType::Model) continue;

            string newResName = CUtil::NewResourceName(loopCount++);
            string modelName(newResName);
            modelName.append("_M");

            romSegments.append("extern u8 _");
            romSegments.append(modelName);
            romSegments.append("SegmentRomStart[];\n");
            romSegments.append("extern u8 _");
            romSegments.append(modelName);
            romSegments.append("SegmentRomEnd[];\n");

            map<string, string> resources = actor->GetResources();
            if (resources.count("textureDataPath"))
            {
                string textureName(newResName);
                textureName.append("_T");

                romSegments.append("extern u8 _");
                romSegments.append(textureName);
                romSegments.append("SegmentRomStart[];\n");
                romSegments.append("extern u8 _");
                romSegments.append(textureName);
                romSegments.append("SegmentRomEnd[];\n");
            }
        }

        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string segmentsPath(buffer);
            segmentsPath.append("\\..\\..\\Engine\\segments.h");
            FILE *file = fopen(segmentsPath.c_str(), "w");
            if (file == NULL) return false;
            fwrite(romSegments.c_str(), 1, romSegments.size(), file);
            fclose(file);
            return true;
        }
        return false;
    }

    bool CBuild::WriteModelsFile(vector<CActor*> actors)
    {
        string modelLoadStart("\nvoid _UER_Load() {");
        const char *modelLoadEnd = "}";
        const char *drawStart = "\n\nvoid _UER_Draw(Gfx **display_list) {";
        const char *drawEnd = "}";
        string modelInits, modelDraws;
        int loopCount = 0;
        char countBuffer[10];
        for (auto actor : actors)
        {
            if (actor->GetType() != ActorType::Model) continue;

            string newResName = CUtil::NewResourceName(loopCount++);
            string modelName(newResName);
            modelName.append("_M");

            _itoa(loopCount - 1, countBuffer, 10);
            modelInits.append("\n\t_UER_Models[");
            modelInits.append(countBuffer);

            map<string, string> resources = actor->GetResources();
            if (resources.count("textureDataPath"))
            {
                modelInits.append("] = (struct actor*)load_model_with_texture(_");
            }
            else {
                modelInits.append("] = (struct actor*)load_model(_");
            }

            modelInits.append(modelName);
            modelInits.append("SegmentRomStart, _");
            modelInits.append(modelName);
            modelInits.append("SegmentRomEnd");

            modelDraws.append("\n\tmodel_draw(_UER_Models[");
            modelDraws.append(countBuffer);
            modelDraws.append("], display_list);\n");

            if (resources.count("textureDataPath"))
            {
                string textureName(newResName);
                textureName.append("_T");

                modelInits.append(", _");
                modelInits.append(textureName);
                modelInits.append("SegmentRomStart, _");
                modelInits.append(textureName);
                modelInits.append("SegmentRomEnd");
            }

            // Add transform data.
            char vectorBuffer[128];
            D3DXVECTOR3 position = actor->GetPosition();
            D3DXVECTOR3 axis;
            float angle;
            D3DXVECTOR3 scale = actor->GetScale();
            actor->GetAxisAngle(&axis, &angle);
            sprintf(vectorBuffer, ", %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf",
                position.x, position.y, position.z,
                axis.x, axis.y, axis.z, angle * (180 / D3DX_PI),
                scale.x, scale.y, scale.z);
            modelInits.append(vectorBuffer);
            modelInits.append(");\n");

            // Write out mesh data.
            vector<Vertex> vertices = actor->GetVertices();
            string id = CUtil::GuidToString(actor->GetId());
            id.insert(0, CUtil::RootPath().append("\\"));
            id.append(".rom.sos");
            FILE *file = fopen(id.c_str(), "w");
            if (file == NULL) return false;
            fprintf(file, "%i\n", vertices.size());
            for (unsigned int i = 0; i < vertices.size(); i++)
            {
                Vertex vert = vertices[i];
                fprintf(file, "%f %f %f %f %f\n",
                    vert.position.x,
                    vert.position.y,
                    vert.position.z,
                    vert.tu,
                    vert.tv);
            }
            fclose(file);
        }

        _itoa(loopCount, countBuffer, 10);
        string modelArray("struct actor *_UER_Models[");
        modelArray.append(countBuffer);
        modelArray.append("];\n");
        modelLoadStart.insert(0, modelArray);

        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string modelInitsPath(buffer);
            modelInitsPath.append("\\..\\..\\Engine\\models.h");
            FILE *file = fopen(modelInitsPath.c_str(), "w");
            if (file == NULL) return false;
            fwrite(modelLoadStart.c_str(), 1, modelLoadStart.size(), file);
            fwrite(modelInits.c_str(), 1, modelInits.size(), file);
            fwrite(modelLoadEnd, 1, strlen(modelLoadEnd), file);
            fwrite(drawStart, 1, strlen(drawStart), file);
            fwrite(modelDraws.c_str(), 1, modelDraws.size(), file);
            fwrite(drawEnd, 1, strlen(drawEnd), file);
            fclose(file);
            return true;
        }
        return false;
    }

    bool CBuild::WriteCamerasFile(vector<CActor*> actors)
    {
        string cameraSetStart("void _UER_Camera() {");
        const char *cameraSetEnd = "}";
        string cameras;
        int cameraCount = 0;
        char countBuffer[10];

        for (auto actor : actors)
        {
            if (actor->GetType() != ActorType::Camera) continue;

            _itoa(cameraCount++, countBuffer, 10);
            cameras.append("\n\t_UER_Cameras[").append(countBuffer).append("] = (struct actor*)create_camera(");

            char vectorBuffer[128];
            D3DXVECTOR3 position = actor->GetPosition();
            D3DXVECTOR3 axis;
            float angle;
            actor->GetAxisAngle(&axis, &angle);
            sprintf(vectorBuffer, "%lf, %lf, %lf, %lf, %lf, %lf, %lf", position.x, position.y, position.z,
                axis.x, axis.y, axis.z, angle * (180 / D3DX_PI));
            cameras.append(vectorBuffer);
            cameras.append(");\n");
        }

        _itoa(cameraCount, countBuffer, 10);
        string cameraArray("struct actor *_UER_Cameras[");
        cameraArray.append(countBuffer);
        cameraArray.append("];\n");
        cameraSetStart.insert(0, cameraArray);

        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string camerasPath(buffer);
            camerasPath.append("\\..\\..\\Engine\\cameras.h");
            FILE *file = fopen(camerasPath.c_str(), "w");
            if (file == NULL) return false;
            fwrite(cameraSetStart.c_str(), 1, cameraSetStart.size(), file);
            fwrite(cameras.c_str(), 1, cameras.size(), file);
            fwrite(cameraSetEnd, 1, strlen(cameraSetEnd), file);
            fclose(file);
            return true;
        }
        return false;
    }

    bool CBuild::WriteCollisionFile(vector<CActor*> actors)
    {
        string collideSetStart("void _UER_Collide() {");
        const char *collideSetEnd = "}";
        string collisions;
        int collisionCount = 0;
        char countBuffer[10];

        for (auto actor : actors)
        {
            int subLoop = collisionCount++;
            for (auto subActor = next(actors.begin(), collisionCount); subActor != actors.end(); ++subActor)
            {
                _itoa(collisionCount-1, countBuffer, 10);
                collisions.append("\n\tcheck_collision(_UER_Actors[").append(countBuffer);
                _itoa(++subLoop, countBuffer, 10);
                collisions.append("], _UER_Actors[").append(countBuffer).append("]);\n");
            }
        }

        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string collisionPath(buffer);
            collisionPath.append("\\..\\..\\Engine\\collision.h");
            FILE *file = fopen(collisionPath.c_str(), "w");
            if (file == NULL) return false;
            fwrite(collideSetStart.c_str(), 1, collideSetStart.size(), file);
            fwrite(collisions.c_str(), 1, collisions.size(), file);
            fwrite(collideSetEnd, 1, strlen(collideSetEnd), file);
            fclose(file);
            return true;
        }
        return false;
    }

    bool CBuild::WriteScriptsFile(vector<CActor*> actors)
    {
        string scriptStartStart("void _UER_Start() {");
        const char *scriptStartEnd = "}";

        string scriptUpdateStart("\n\nvoid _UER_Update() {");
        const char *scriptUpdateEnd = "}";

        string inputStart("\n\nvoid _UER_Input(NUContData gamepads[4]) {");
        const char *inputEnd = "}";

        string scripts;
        char countBuffer[10];
        int loopCount = 0, modelCount = 0, cameraCount = 0;

        for (auto actor : actors)
        {
            string actorRef;
            int index = 0;

            if (actor->GetType() == ActorType::Model)
            {
                actorRef.append("_UER_Models[");
                index = modelCount++;
            }
            else
            {
                actorRef.append("_UER_Cameras[");
                index = cameraCount++;
            }

            string newResName = CUtil::NewResourceName(loopCount++);
            string script = actor->GetScript();
            char *result = CUtil::ReplaceString(script.c_str(), "@", newResName.c_str());

            _itoa(index, countBuffer, 10);
            actorRef.append(countBuffer).append("]->");
            result = CUtil::ReplaceString(result, "self->", actorRef.c_str());
            scripts.append(result).append("\n\n");

            if (scripts.find(string(newResName).append("start(")) != string::npos)
            {
                scriptStartStart.append("\n\t").append(newResName).append("start();\n");
            }

            if (scripts.find(string(newResName).append("update(")) != string::npos)
            {
                scriptUpdateStart.append("\n\t").append(newResName).append("update();\n");
            }

            if (scripts.find(string(newResName).append("input(")) != string::npos)
            {
                inputStart.append("\n\t").append(newResName).append("input(gamepads);\n");
            }

            free(result);
        }

        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string scriptsPath(buffer);
            scriptsPath.append("\\..\\..\\Engine\\scripts.h");
            FILE *file = fopen(scriptsPath.c_str(), "w");
            if (file == NULL) return false;
            fwrite(scripts.c_str(), 1, scripts.size(), file);
            fwrite(scriptStartStart.c_str(), 1, scriptStartStart.size(), file);
            fwrite(scriptStartEnd, 1, strlen(scriptStartEnd), file);
            fwrite(scriptUpdateStart.c_str(), 1, scriptUpdateStart.size(), file);
            fwrite(scriptUpdateEnd, 1, strlen(scriptUpdateEnd), file);
            fwrite(inputStart.c_str(), 1, inputStart.size(), file);
            fwrite(inputEnd, 1, strlen(inputEnd), file);
            fclose(file);
            return true;
        }
        return false;
    }

    bool CBuild::WriteMappingsFile(vector<CActor*> actors)
    {
        string mappingsStart("void _UER_Mappings() {");
        const char *mappingsEnd = "\n}";

        int loopCount = 0;
        char countBuffer[10];

        for (auto actor : actors)
        {
            _itoa(loopCount++, countBuffer, 10);
            mappingsStart.append("\n\tinsert(\"").append(actor->GetName()).append("\", ").append(countBuffer).append(");");
        }

        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string mappingsPath(buffer);
            mappingsPath.append("\\..\\..\\Engine\\mappings.h");
            FILE *file = fopen(mappingsPath.c_str(), "w");
            if (file == NULL) return false;
            fwrite(mappingsStart.c_str(), 1, mappingsStart.size(), file);
            fwrite(mappingsEnd, 1, strlen(mappingsEnd), file);
            fclose(file);
            return true;
        }
        return false;
    }

    bool CBuild::Start(vector<CActor*> actors)
    {
        WriteSpecFile(actors);
        WriteSegmentsFile(actors);
        WriteModelsFile(actors);
        WriteCamerasFile(actors);
        WriteCollisionFile(actors);
        WriteScriptsFile(actors);
        WriteMappingsFile(actors);
        return Compile();
    }

    bool CBuild::Run()
    {
        // Get the path to where the program is running.
        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            DWORD exitCode;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            // Format the path to execute the ROM build.
            string currDir(buffer);
            currDir.append("\\..\\..\\Player");

            // Start the build with no window.
            CreateProcess(NULL, "cmd /c cen64.exe pifdata.bin ..\\Engine\\main.n64", NULL, NULL, FALSE,
                CREATE_NO_WINDOW, NULL, currDir.c_str(), &si, &pi);

            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return exitCode == 0;
        }

        return false;
    }

    bool CBuild::Load()
    {
        // Get the path to where the program is running.
        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            DWORD exitCode;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            // Format the path to execute the ROM build.
            string currDir(buffer);
            currDir.append("\\..\\..\\Player\\USB");

            // Start the USB loader with no window.
            CreateProcess(NULL, "cmd /c 64drive_usb.exe -l ..\\..\\Engine\\main.n64 -c 6102", NULL, NULL, FALSE,
                CREATE_NO_WINDOW, NULL, currDir.c_str(), &si, &pi);

            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return exitCode == 0;
        }

        return false;
    }

    bool CBuild::Compile()
    {
        // Set the root env variable for the N64 build tools.
        SetEnvironmentVariable("ROOT", "..\\Engine\\n64sdk\\ultra");

        // Get the path to where the program is running.
        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            DWORD exitCode;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            // Format the path to execute the ROM build.
            string currDir(buffer);
            currDir.append("\\..\\..\\Engine");

            // Start the build with no window.
            CreateProcess(NULL, "cmd /c build.bat", NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, currDir.c_str(), &si, &pi);
            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return exitCode == 0;
        }

        return false;
    }
}
