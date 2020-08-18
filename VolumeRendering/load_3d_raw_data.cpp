GLubyte * load_3d_raw_data(std::string texture_path, glm::vec3 dimension) {
	size_t size = dimension[0] * dimension[1] * dimension[2];

	FILE *fp;
	GLubyte *data = new GLubyte[size];			  // 8bit
	if (!(fp = fopen(texture_path.c_str(), "rb"))) {
		std::cout << "Error: opening .raw file failed" << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "OK: open .raw file successed" << std::endl;
	}
	if (fread(data, sizeof(char), size, fp) != size) {
		std::cout << "Error: read .raw file failed" << std::endl;
		exit(1);
	}
	else {
		std::cout << "OK: read .raw file successed" << std::endl;
	}
	fclose(fp);
	return data;
}