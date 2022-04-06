uniform vec3 lightDirection = vec3(0,0,1);

vec3 dsModel(const in vec3 pos, const in vec3 n)
{
    // Calculate the vector from the light to the fragment
    vec3 s = lightDirection;//normalize(lightPosition - pos);

    // Calculate the vector from the fragment to the eye position
    // (origin since this is in "eye" or "camera" space)
    vec3 v = lightDirection;//normalize(-pos);

    // Reflect the light beam using the normal at this fragment
    vec3 r = reflect(-s, n);

    // Calculate the diffuse component
    float diffuse = dot(s, n);

    // Calculate the specular component
    float specular = diffuse > 0.0 ? pow(max(dot(r, v), 0.0), 60) : 0.0;

    // Combine the ambient, diffuse and specular contributions
    return vec3(1.0, abs(diffuse), specular);
}
