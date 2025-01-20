import os
# pip install PyPDF2 python-docx
from PyPDF2 import PdfReader
from docx import Document

def read_file(file_path):
    """
    Read content from PDF, DOCX, or TXT files
    Returns the text content as string
    """
    try:
        # Get file extension
        file_ext = os.path.splitext(file_path)[1].lower()
        
        # Read PDF files
        if file_ext == '.pdf':
            reader = PdfReader(file_path)
            text = ''
            for page in reader.pages:
                text += page.extract_text()
            return text
            
        # Read DOCX files    
        elif file_ext == '.docx':
            doc = Document(file_path)
            text = ''
            for paragraph in doc.paragraphs:
                text += paragraph.text + '\n'
            return text
            
        # Read TXT files
        elif file_ext == '.txt':
            with open(file_path, 'r', encoding='utf-8') as file:
                return file.read()
                
        else:
            raise ValueError(f"Unsupported file format: {file_ext}")
            
    except Exception as e:
        raise Exception(f"Error reading file: {str(e)}")