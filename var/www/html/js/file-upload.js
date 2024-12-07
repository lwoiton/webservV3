// File upload and listing functionality
document.addEventListener('DOMContentLoaded', () => {
    const fileInput = document.getElementById('fileInput');
    const selectedFile = document.querySelector('.selected-file');
    const uploadForm = document.getElementById('uploadForm');
    const previewModal = document.getElementById('previewModal');
    
    // File input change handler
    fileInput.addEventListener('change', () => {
        selectedFile.textContent = fileInput.files[0] ? fileInput.files[0].name : 'No file selected';
    });

    // Form submit handler
    uploadForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        
        if (!fileInput.files[0]) {
            alert('Please select a file first');
            return;
        }

        const formData = new FormData();
        formData.append('file', fileInput.files[0]);

        try {
            const response = await fetch('/upload', {
                method: 'POST',
                body: formData
            });

            if (!response.ok) throw new Error('Upload failed');

            // Clear form and refresh file list
            fileInput.value = '';
            selectedFile.textContent = 'No file selected';
            fetchFileList();
            
        } catch (error) {
            alert('Upload failed: ' + error.message);
        }
    });

    // File preview handler
    function showPreview(file) {
        const previewContent = document.getElementById('previewContent');
        const extension = file.name.split('.').pop().toLowerCase();
        
        // Clear previous content
        previewContent.innerHTML = '';
        
        if (['jpg', 'jpeg', 'png', 'gif'].includes(extension)) {
            // Image preview
            const img = document.createElement('img');
            img.src = file.url;
            img.style.maxWidth = '100%';
            previewContent.appendChild(img);
        } else if (['txt', 'md', 'csv'].includes(extension)) {
            // Text preview
            fetch(file.url)
                .then(response => response.text())
                .then(text => {
                    const pre = document.createElement('pre');
                    pre.textContent = text;
                    previewContent.appendChild(pre);
                });
        } else {
            previewContent.innerHTML = '<p>Preview not available for this file type</p>';
        }
        
        previewModal.style.display = 'flex';
    }

    // Close preview modal
    document.querySelector('.close-preview').addEventListener('click', () => {
        previewModal.style.display = 'none';
    });

    // File list handler
    async function fetchFileList() {
        try {
            const response = await fetch('/files/');
            const data = await response.json();
            
            const fileList = document.getElementById('fileList');
            fileList.innerHTML = '';
            
            data.files.forEach(file => {
                const row = document.createElement('tr');
                row.innerHTML = `
                    <td>
                        <a href="#" class="file-name" data-file='${JSON.stringify(file)}'>
                            <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                <path d="M13 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V9z"></path>
                                <polyline points="13 2 13 9 20 9"></polyline>
                            </svg>
                            ${file.name}
                        </a>
                    </td>
                    <td>${formatFileSize(file.size)}</td>
                    <td>${new Date(file.modified).toLocaleDateString()}</td>
                    <td>
                        <button class="delete-btn" onclick="deleteFile('${file.name}')">
                            <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                <polyline points="3 6 5 6 21 6"></polyline>
                                <path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"></path>
                            </svg>
                        </button>
                    </td>
                `;
                
                // Add click handler for preview
                row.querySelector('.file-name').addEventListener('click', (e) => {
                    e.preventDefault();
                    const fileData = JSON.parse(e.currentTarget.dataset.file);
                    showPreview(fileData);
                });
                
                fileList.appendChild(row);
            });
        } catch (error) {
            console.error('Error fetching file list:', error);
        }
    }

    // Utility function to format file sizes
    function formatFileSize(bytes) {
        if (bytes === 0) return '0 Bytes';
        const k = 1024;
        const sizes = ['Bytes', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
    }

    // Delete file handler
    window.deleteFile = async (filename) => {
        if (!confirm(`Are you sure you want to delete ${filename}?`)) return;
        
        try {
            const response = await fetch(`/files/${encodeURIComponent(filename)}`, {
                method: 'DELETE'
            });
            
            if (!response.ok) throw new Error('Delete failed');
            
            fetchFileList();
        } catch (error) {
            alert('Delete failed: ' + error.message);
        }
    };

    // Initial file list load
    fetchFileList();
});