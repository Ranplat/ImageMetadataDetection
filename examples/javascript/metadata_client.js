/**
 * 图像元数据检测与取证分析系统客户端
 */
class ImageForensicsClient {
  /**
   * 初始化客户端
   * @param {string} baseUrl - API服务的基础URL
   */
  constructor(baseUrl = 'http://localhost:8080') {
    this.baseUrl = baseUrl;
  }

  /**
   * 检查API服务健康状态
   * @returns {Promise<Object>} 包含健康状态的对象
   */
  async checkHealth() {
    try {
      const response = await fetch(`${this.baseUrl}/health`);
      return await response.json();
    } catch (error) {
      return { status: 'error', message: error.message };
    }
  }

  /**
   * 提取图像元数据
   * @param {File} imageFile - 图像文件对象
   * @returns {Promise<Object>} 包含元数据的对象
   */
  async extractMetadata(imageFile) {
    try {
      const formData = new FormData();
      formData.append('image', imageFile);

      const response = await fetch(`${this.baseUrl}/metadata`, {
        method: 'POST',
        body: formData
      });

      return await response.json();
    } catch (error) {
      return { status: 'error', message: error.message };
    }
  }

  /**
   * 批量提取多个图像的元数据
   * @param {Array<File>} imageFiles - 图像文件对象数组
   * @returns {Promise<Object>} 包含所有图像元数据的对象
   */
  async batchExtractMetadata(imageFiles) {
    try {
      const formData = new FormData();
      
      imageFiles.forEach(file => {
        formData.append('images[]', file);
      });

      const response = await fetch(`${this.baseUrl}/metadata/batch`, {
        method: 'POST',
        body: formData
      });

      return await response.json();
    } catch (error) {
      return { status: 'error', message: error.message };
    }
  }

  /**
   * 分析图像是否被篡改
   * @param {File} imageFile - 图像文件对象
   * @returns {Promise<Object>} 包含取证分析结果的对象
   */
  async analyzeForensics(imageFile) {
    try {
      const formData = new FormData();
      formData.append('image', imageFile);

      const response = await fetch(`${this.baseUrl}/forensics`, {
        method: 'POST',
        body: formData
      });

      return await response.json();
    } catch (error) {
      return { status: 'error', message: error.message };
    }
  }
}

// 在浏览器环境中使用
if (typeof window !== 'undefined') {
  window.ImageForensicsClient = ImageForensicsClient;
}

// 在Node.js环境中使用
if (typeof module !== 'undefined' && module.exports) {
  module.exports = ImageForensicsClient;
}

/**
 * 使用示例（浏览器环境）
 */
async function browserExample() {
  // 创建客户端实例
  const client = new ImageForensicsClient();
  
  // 检查API健康状态
  const healthStatus = await client.checkHealth();
  console.log('API健康状态:', healthStatus);
  
  // 获取文件输入元素
  const fileInput = document.getElementById('imageFile');
  if (fileInput && fileInput.files.length > 0) {
    const imageFile = fileInput.files[0];
    
    // 提取元数据
    console.log(`从 ${imageFile.name} 提取元数据...`);
    const metadata = await client.extractMetadata(imageFile);
    console.log('元数据结果:', metadata);
    
    // 取证分析
    console.log(`对 ${imageFile.name} 进行取证分析...`);
    const forensics = await client.analyzeForensics(imageFile);
    console.log('取证分析结果:', forensics);
    
    // 判断图像是否被篡改
    if (forensics.status === 'success') {
      const isTampered = forensics.forensics?.is_tampered;
      if (isTampered) {
        console.warn('警告: 图像可能已被篡改!');
        const indicators = forensics.forensics?.tampering_indicators || [];
        indicators.forEach((indicator, index) => {
          console.warn(`篡改指标 ${index + 1}: ${indicator.description}`);
        });
      } else {
        console.log('图像未检测到篡改痕迹');
      }
    }
  }
} 