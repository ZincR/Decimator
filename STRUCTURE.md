# Decimator Project Structure

## Overview

A visualized, customizable mesh simplifier implementing three different simplification algorithms with interactive controls.

## Directory Structure

```
Decimator/
├── CMakeLists.txt                          # Build configuration
├── README.txt                              # Project overview
├── STRUCTURE.md                            # This file
├── gloo.cfg                                # GLOO configuration
│
├── assets/
│   └── decimator/                          # Mesh files (.obj)
│
└── assignment_code/
    ├── common/
    │   ├── helpers.hpp                     # Shared utility functions
    │   └── helpers.cpp
    │
    └── decimator/
        ├── main.cpp                        # Application entry point
        ├── DecimatorApp.hpp/cpp            # Main application class
        ├── MeshSimplifierNode.hpp/cpp      # Main orchestration node
        │
        ├── LightNode.hpp/cpp               # Lighting
        ├── DirectionalLight.hpp            # Directional light implementation
        │
        ├── MeshIO.hpp/cpp                  # OBJ file loading/saving
        ├── MeshSelection.hpp/cpp           # Ray-based selection system
        ├── WireframeRenderer.hpp/cpp       # Wireframe/vertex visualization
        │
        ├── simplification/
        │   ├── MeshData.hpp/cpp            # Core mesh data structure
        │   ├── EdgeCollapse.hpp/cpp        # Garland-Heckbert algorithm
        │   ├── VertexDecimation.hpp/cpp    # Schroeder-Zarge-Lorensen
        │   └── VertexClustering.hpp/cpp    # Rossignac-Borrel
        │
        └── README.txt                      # Assignment-specific notes
```

## Component Details

### 1. Core Application (`main.cpp`, `DecimatorApp`)

- Application initialization and main loop
- Scene setup with camera, lights, and mesh simplifier node

### 2. MeshSimplifierNode (Main Orchestrator)

**Responsibilities:**

- Manages three simplification methods
- Handles keyboard input for method toggling
- Provides ImGui interface for controls
- Coordinates rendering and selection

**Key Controls:**

- `1-3`: Switch between simplification methods
- `O`: Toggle original mesh display
- `W`: Toggle wireframe mode
- `V`: Toggle vertex display as dots
- `S`: Save current mesh to OBJ file
- `L`: Load mesh from file
- `R`: Run simplification with current method
- `A`: Run all three simplification methods

**Features:**

- Side-by-side comparison of methods
- Real-time parameter adjustment
- Mesh statistics display
- Transform controls (position, rotation, scale)

### 3. Simplification Methods

#### 3.1 Edge Collapse (Garland-Heckbert)

**File:** `simplification/EdgeCollapse.hpp/cpp`

**Algorithm Structure:**

- Quadric error metric computation
- Priority queue-based edge selection
- Optimal vertex position calculation
- Iterative edge collapse

**Key Components:**

- `QuadricMatrix`: 4x4 symmetric matrix for error metric
- `Edge`: Edge structure with collapse error
- Methods for quadric computation and edge collapse

#### 3.2 Vertex Decimation (Schroeder-Zarge-Lorensen)

**File:** `simplification/VertexDecimation.hpp/cpp`

**Algorithm Structure:**

- Vertex classification (feature, boundary, interior)
- Distance error computation
- Vertex removal with retriangulation
- Aspect ratio and feature preservation

**Key Components:**

- `VertexInfo`: Classification and error data
- Feature angle threshold
- Hole retriangulation methods

#### 3.3 Vertex Clustering (Rossignac-Borrel)

**File:** `simplification/VertexClustering.hpp/cpp`

**Algorithm Structure:**

- Uniform grid overlay
- Vertex assignment to cells
- Representative vertex computation
- Cluster merging

**Key Components:**

- `GridCell`: Cell structure with vertex list
- Bounding box computation
- Grid resolution control

### 4. MeshData Structure

**File:** `simplification/MeshData.hpp/cpp`

**Purpose:** Universal mesh representation for algorithms

**Data:**

- Vertex positions (`std::vector<glm::vec3>`)
- Face indices (`std::vector<glm::uvec3>`)
- Normals, colors, texture coordinates (optional)

**Methods:**

- `FromVertexObject()`: Convert from GLOO format
- `ToVertexObject()`: Convert to GLOO format for rendering
- `ComputeNormals()`: Calculate vertex normals
- `Validate()`: Check mesh integrity

### 5. Selection System

**File:** `MeshSelection.hpp/cpp`

**Features:**

- Ray-mesh intersection (Möller-Trumbore algorithm)
- Vertex/edge selection via ray casting
- Region selection (radius-based)
- Selected element deletion
- Localized simplification

**Key Components:**

- `Ray`: Ray structure for picking
- `IntersectionResult`: Hit information
- Selection management (vertices and edges)

### 6. Wireframe Renderer

**File:** `WireframeRenderer.hpp/cpp`

**Render Modes:**

- `WIREFRAME`: Edges only
- `VERTICES`: Vertices as dots
- `WIREFRAME_VERTICES`: Both edges and dots
- `SOLID`: Standard solid rendering
- `SOLID_WIREFRAME`: Solid with wireframe overlay

**Features:**

- Configurable vertex size and edge width
- Color customization
- Selection highlighting

### 7. File I/O

**File:** `MeshIO.hpp/cpp`

**Capabilities:**

- OBJ file loading with format support
- OBJ file saving
- Integration with GLOO's MeshLoader
- Face triangulation for complex polygons

## Implementation Workflow

### Phase 1: Setup (Complete)

✅ Project structure
✅ Build system
✅ Core application skeleton
✅ UI framework
✅ Key binding system

### Phase 2: Simplification Algorithms (To Implement)

Each algorithm file contains:

- Detailed structure and method signatures
- Comments indicating TODOs
- Helper method skeletons

**Recommended Implementation Order:**

1. **Vertex Clustering** (simplest)
   - Grid-based approach
   - Good for testing pipeline
2. **Vertex Decimation** (medium)
   - More sophisticated criteria
   - Introduces feature preservation
3. **Edge Collapse** (most complex)
   - Quadric error metrics
   - Optimal vertex placement

### Phase 3: Selection & Interaction (To Implement)

- Complete ray-casting implementation
- Integrate with mouse input
- Implement deletion operations
- Regional simplification

### Phase 4: Rendering Enhancements (To Implement)

- Complete wireframe shader
- Vertex point rendering
- Selection highlighting
- Performance optimization

## Building the Project

### Prerequisites

Before building, you need:

1. Copy `gloo/` directory from another assignment (e.g., `assignment0`)
2. Copy `external/` directory from another assignment

### Build Steps

```bash
# From Decimator directory
cmake -S . -B build
cd build
make
./decimator
```

## Usage Guide

### Basic Workflow

1. **Load a mesh**: Place OBJ file in `assets/decimator/` or use 'L' key
2. **Adjust reduction**: Use slider (0.01-0.99 = keep 1%-99% of vertices)
3. **Choose method**: Press 1, 2, or 3 for different algorithms
4. **Simplify**: Press 'R' for current method or 'A' for all three
5. **Compare**: Toggle between original ('O') and simplified
6. **Visualize**: Toggle wireframe ('W') and vertices ('V')
7. **Save result**: Press 'S' to export to OBJ

### Advanced Features

- **Selection mode**: Enable in UI to select vertices/edges
- **Regional simplification**: Select area, then simplify
- **Method comparison**: Run all three, toggle between them
- **Transform mesh**: Use ImGui sliders or arrow keys for light

## Next Steps

### Immediate Tasks

1. Implement simplification algorithms in order:

   - Start with `VertexClustering.cpp`
   - Then `VertexDecimation.cpp`
   - Finally `EdgeCollapse.cpp`

2. Complete and connect the mesh selection pipeline:

   - Implement mouse-to-ray conversion in `HandleMouseSelection()`
   - Complete ray-triangle intersection in `MeshSelection.cpp`
   - Connect selection UI controls to actual functionality
   - Implement `DeleteSelectedElements()` to remove selected vertices/edges
   - Implement `SimplifySelectedRegion()` to apply algorithms to selected areas only
   - Add visual highlighting for selected elements
   - Polish the selection workflow (click to select, shift-click for multi-select, etc.)

3. Test with simple meshes:
   - Cube, sphere, teapot
   - Verify each algorithm works

### Enhancement Ideas

- Progressive mesh simplification
- Error metric visualization
- Undo/redo functionality
- Batch processing multiple meshes
- Comparison metrics (error, runtime)
- Different quadric error formulations
- Custom selection brushes
- Texture coordinate preservation

## Notes for Implementation

### Important Considerations

1. **Edge Case Handling**: Check for degenerate faces, invalid indices
2. **Memory Management**: Use smart pointers, avoid deep copies
3. **Performance**: Consider spatial data structures for large meshes
4. **Validation**: Test `MeshData::Validate()` after operations
5. **Normals**: Recompute after simplification

### Debugging Tips

- Start with small test meshes
- Print vertex/face counts at each step
- Visualize intermediate results
- Use wireframe mode to spot issues
- Check for manifold mesh assumptions

### Testing Strategy

1. Unit test each algorithm independently
2. Compare results with reference implementations
3. Test on various mesh complexities
4. Verify file I/O round-trips correctly
5. Stress test with large meshes

## References

- [GH97] Garland & Heckbert, "Surface Simplification Using Quadric Error Metrics"
- [SZL92] Schroeder, Zarge, Lorensen, "Decimation of Triangle Meshes"
- Rossignac & Borrel, "Multi-resolution 3D approximations for rendering"

## Contact & Support

For questions or issues, refer to the course materials or consult with the instructor.
