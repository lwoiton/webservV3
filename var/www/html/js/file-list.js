// file-list.js
async function fetchFileList() {
    try {
        const response = await fetch('/files/');
        const html = await response.text();
        
        // Parse the directory listing
        const parser = new DOMParser();
        const doc = parser.parseFromString(html, 'text/html');
        
        // Extract file links
        const files = Array.from(doc.querySelectorAll('a'))
            .filter(a => !a.href.endsWith('/'))  // Filter out directory links
            .map(a => ({
                name: decodeURIComponent(a.textContent),
                url: a.href
            }));
            
        displayFileList(files);
    } catch (error) {
        console.error('Error fetching file list:', error);
        document.getElementById('fileList').innerHTML = 
            '<p class="error-message">Failed to load file list</p>';
    }
}

function displayFileList(files) {
    const fileList = document.getElementById('fileList');
    
    if (!files.length) {
        fileList.innerHTML = '<p>No files uploaded yet</p>';
        return;
    }

    const html = files.map(file => `
        <div class="file-item">
            <span>${file.name}</span>
            <div class="file-actions">
                <a href="${file.url}" class="cta" download>Download</a>
                <button onclick="deleteFile('${file.name}')" class="cta delete">Delete</button>
            </div>
        </div>
    `).join('');

    fileList.innerHTML = html;
}

async function deleteFile(filename) {
    if (!confirm(`Are you sure you want to delete ${filename}?`)) {
        return;
    }
    
    try {
        const response = await fetch(`/files/${encodeURIComponent(filename)}`, {
            method: 'DELETE'
        });
        
        if (!response.ok) {
            throw new Error(`Delete failed: ${response.statusText}`);
        }
        
        // Refresh the file list
        fetchFileList();
        
    } catch (error) {
        console.error('Delete error:', error);
        alert(`Failed to delete ${filename}: ${error.message}`);
    }
}

// Initial load
document.addEventListener('DOMContentLoaded', fetchFileList);