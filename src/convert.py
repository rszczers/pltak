import xmltodict

with open('../data/codes.xsd') as infile:
    doc = xmltodict.parse(infile.read())

lista = doc['xsd:schema']['xsd:simpleType']['xsd:restriction']['xsd:enumeration']
with open('us_codes.dat', 'w') as outfile:
    for a in lista:
        outfile.write(a['@value'] + ' ' + a['xsd:annotation']['xsd:documentation'] + '\n')

