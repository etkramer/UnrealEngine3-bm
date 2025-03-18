class RAnimNode_AnimCapture extends AnimNodeBlendBase
    native;

var array<BoneAtom> SavedAtoms;

// Export URAnimNode_AnimCapture::execCapture(FFrame&, void* const)
native function Capture(SkeletalMeshComponent Mesh);
