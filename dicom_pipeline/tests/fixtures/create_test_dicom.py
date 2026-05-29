#!/usr/bin/env python3
import os
import numpy as np
from pydicom.dataset import Dataset, FileMetaDataset
from pydicom.uid import ExplicitVRLittleEndian, SecondaryCaptureImageStorage

def create_synthetic_dicom(dest_path):
    os.makedirs(os.path.dirname(dest_path), exist_ok=True)

    # 1. File Meta Information
    file_meta = FileMetaDataset()
    file_meta.FileMetaInformationGroupLength = 0  # will be computed
    file_meta.FileMetaInformationVersion = b'\x00\x01'
    file_meta.MediaStorageSOPClassUID = SecondaryCaptureImageStorage
    file_meta.MediaStorageSOPInstanceUID = '1.2.3.4.5.6.7.8.9'
    file_meta.TransferSyntaxUID = ExplicitVRLittleEndian
    file_meta.ImplementationClassUID = '1.2.840.10008.1.2'

    # 2. Main Dataset
    ds = Dataset()
    ds.file_meta = file_meta
    ds.is_little_endian = True
    ds.is_implicit_VR = False

    # Standard Mandatory DICOM Tags
    ds.SOPClassUID = SecondaryCaptureImageStorage
    ds.SOPInstanceUID = '1.2.3.4.5.6.7.8.9'
    ds.PatientName = 'Test^Patient'
    ds.PatientID = 'PAT001'
    ds.PatientBirthDate = '19900101'
    ds.PatientSex = 'M'
    ds.InstitutionName = 'Test Hospital'
    ds.ReferringPhysicianName = 'Dr. Test'
    ds.Modality = 'CT'
    ds.StudyDate = '20260529'
    ds.StudyTime = '120000'

    # Image Pixel Tags
    ds.Rows = 64
    ds.Columns = 64
    ds.SamplesPerPixel = 1
    ds.PhotometricInterpretation = 'MONOCHROME2'
    ds.PixelRepresentation = 0
    ds.BitsAllocated = 16
    ds.BitsStored = 16
    ds.HighBit = 15

    # Generate synthetic Gaussian pixel values upcast to uint16
    mean, std = 1000, 200
    pixels = np.random.normal(mean, std, (64, 64)).astype(np.uint16)
    ds.PixelData = pixels.tobytes()

    ds.save_as(dest_path, write_like_original=False)
    print(f"Created synthetic DICOM fixture at {dest_path}")

if __name__ == '__main__':
    base_dir = os.path.dirname(os.path.abspath(__file__))
    dest = os.path.join(base_dir, 'sample_64x64.dcm')
    create_synthetic_dicom(dest)
